import os
import subprocess
import sys
import json
import time

# --- ANSI Colors ---
C = '\033[96m'  # Cyan
G = '\033[92m'  # Green
Y = '\033[93m'  # Yellow
R = '\033[91m'  # Red
W = '\033[0m'   # Reset/White
B = '\033[1m'   # Bold

def run_command(command):
    try:
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end='')
        process.wait()
        return process.returncode
    except Exception as e:
        print(f"{R}[!] Error al ejecutar el comando: {e}{W}")
        return 1

def check_pio():
    print(f"{C}--- Verificando PlatformIO ---{W}")
    # Try 'pio' first
    result = subprocess.run("pio --version", shell=True, capture_output=True)
    if result.returncode == 0:
        return "pio"
    
    # Try 'python -m platformio'
    result = subprocess.run(f"{sys.executable} -m platformio --version", shell=True, capture_output=True)
    if result.returncode == 0:
        return f"{sys.executable} -m platformio"
    
    print(f"\n{R}[!] Error: PlatformIO no está instalado o no está en el PATH.{W}")
    print(f"Instálalo con: {Y}pip install -U platformio{W}")
    return None

def detect_boards(pio_cmd):
    print(f"{C}--- Buscando tarjetas conectadas ---{W}")
    try:
        result = subprocess.run(f"{pio_cmd} device list --json-output", shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            devices = json.loads(result.stdout)
            esp_devices = []
            for d in devices:
                desc = d.get('description', '').lower()
                hwid = d.get('hwid', '').lower()
                # Common identifiers for ESP32 and USB-Serial bridges
                if any(x in desc or x in hwid for x in ['cp210', 'ch34', 'esp32', 'silicon labs', '10c4:ea60', '1a86:7523']):
                    esp_devices.append(d)
            return esp_devices
    except:
        pass
    return []

def get_chip_info(port):
    try:
        cmd = f"{sys.executable} -m esptool --port {port} chip-id"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            # Extraer solo las lineas relevantes
            lines = result.stdout.split('\n')
            info = {}
            for line in lines:
                if "Chip is" in line: info['chip'] = line.strip()
                if "MAC:" in line: info['mac'] = line.replace("MAC: ", "").strip()
                if "Features:" in line: info['features'] = line.replace("Features: ", "").strip()
            
            if 'chip' in info:
                print(f"  {B}Chip:{W} {info['chip']}")
                print(f"  {B}MAC:{W}  {info['mac']}")
                return info
    except:
        pass
    return None

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

def main():
    clear_screen()
    print(f"{C}{B}========================================{W}")
    print(f"{C}{B}     Jmouse HzPro - Flash Tool 🚀{W}")
    print(f"{C}{B}========================================{W}\n")

    pio_cmd = check_pio()
    if not pio_cmd:
        sys.exit(1)

    detected_info = None
    boards = detect_boards(pio_cmd)
    if boards:
        print(f"{G}Se encontraron {len(boards)} tarjeta(s):{W}")
        for b in boards:
            print(f"  {G}[+]{W} {B}{b['port']}{W} - {b['description']}")
            detected_info = get_chip_info(b['port'])
    else:
        print(f"{Y}[!] No se detectaron tarjetas automáticamente. Verifica el cable USB.{W}")

    environments = [
        {"id": "esp32dev", "name": "Solo Bluetooth (ESP32 Clásico)", "desc": "Para WROOM/WROVER. No soporta USB HID."},
        {"id": "esp32s3_ble", "name": "Dual: Bluetooth + USB (ESP32-S3)", "desc": "RECOMENDADO. Soporta ambos simultáneamente."},
        {"id": "esp32s3_usb", "name": "Solo USB Directo (ESP32-S3)", "desc": "Desactiva Bluetooth para máxima discreción."},
        {"id": "esp32s2", "name": "Solo USB Directo (ESP32-S2)", "desc": "Para tarjetas S2. No tiene Bluetooth."},
    ]

    while True:
        print(f"\n{C}{B}--- MENÚ PRINCIPAL ---{W}")
        print("🛠️  Opciones de Programación:")
        for i, env in enumerate(environments, 1):
            print(f"  {B}{i}.{W} Flashear: {env['name']}")
            print(f"     {Y}↳ {env['desc']}{W}")
        
        print("\n🔍 Opciones de Diagnóstico:")
        print(f"  {B}5.{W} Abrir Monitor Serie (Ver logs en tiempo real)")
        print(f"  {B}6.{W} Salir")

        try:
            choice = input(f"\n{B}Elige una opción (1-6): {W}")
            
            if choice == '6' or choice.lower() == 'q':
                print(f"{G}¡Hasta luego!{W}")
                sys.exit(0)
            
            if choice == '5':
                print(f"\n{C}--- Iniciando Monitor Serie (115200 baudios) ---{W}")
                print(f"{Y}--- Presiona Ctrl+C para salir del monitor ---{W}")
                time.sleep(1)
                os.system(f"{pio_cmd} device monitor -b 115200")
                continue
                
            idx = int(choice) - 1
            if 0 <= idx < len(environments):
                target = environments[idx]["id"]
                selected_env = environments[idx]
                print(f"\n{C}>>> Iniciando programación para: {B}{selected_env['name']}{W}...")
                
                # Compilar y subir
                cmd = f"{pio_cmd} run -e {target} -t upload"
                result = run_command(cmd)
                
                if result == 0:
                    print(f"\n{G}{B}" + "="*40 + f"{W}")
                    print(f"{G}{B}       RESUMEN DE PROGRAMACIÓN{W}")
                    print(f"{G}{B}" + "="*40 + f"{W}")
                    print(f" ESTADO:    {G}EXITOSO ✅{W}")
                    print(f" MODO:      {C}{selected_env['name']}{W}")
                    if detected_info:
                        print(f" CHIP:      {detected_info.get('chip', 'Desconocido')}")
                        print(f" MAC:       {detected_info.get('mac', 'Desconocido')}")
                    print("-" * 40)
                    print(f"{B} PRÓXIMOS PASOS:{W}")
                    print(f" 1. Busca el WiFi: {C}JmouseHzPro-Setup{W}")
                    print(f" 2. Ve a: {C}http://192.168.4.1{W}")
                    print(f" 3. Configura tus patrones de movimiento")
                    print(f"{G}{B}" + "="*40 + f"{W}")
                    
                    mon = input(f"\n¿Quieres abrir el Monitor Serie ahora? (s/n): ")
                    if mon.lower() == 's':
                        print(f"\n{C}--- Iniciando Monitor (115200 baudios) ---{W}")
                        print(f"{Y}--- Presiona Ctrl+C para salir del monitor ---{W}")
                        time.sleep(1)
                        os.system(f"{pio_cmd} device monitor -b 115200")
                else:
                    print(f"\n{R}[!] Error durante la programación.{W}")
                    print(f"Tips: Prueba manteniendo el botón {B}BOOT{W} presionado al conectar el USB.")
                
                input(f"\nPresiona Enter para continuar...")
                clear_screen()
            else:
                print(f"{R}Opción no válida. Intenta de nuevo.{W}")
        except ValueError:
            print(f"{R}Entrada no válida.{W}")
        except KeyboardInterrupt:
            print(f"\n{Y}Cancelado por el usuario.{W}")
            sys.exit(0)

if __name__ == "__main__":
    # Activar colores ANSI en la terminal de Windows
    if os.name == 'nt':
        os.system('color')
    main()
