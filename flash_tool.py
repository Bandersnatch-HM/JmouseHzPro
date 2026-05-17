import os
import subprocess
import sys
import json

def run_command(command):
    try:
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end='')
        process.wait()
        return process.returncode
    except Exception as e:
        print(f"Error al ejecutar el comando: {e}")
        return 1

def check_pio():
    print("--- Verificando PlatformIO ---")
    # Try 'pio' first
    result = subprocess.run("pio --version", shell=True, capture_output=True)
    if result.returncode == 0:
        return "pio"
    
    # Try 'python -m platformio'
    result = subprocess.run(f"{sys.executable} -m platformio --version", shell=True, capture_output=True)
    if result.returncode == 0:
        return f"{sys.executable} -m platformio"
    
    print("\n[!] Error: PlatformIO no esta instalado o no esta en el PATH.")
    print("Instalalo con: pip install -U platformio")
    return None

def detect_boards(pio_cmd):
    print("--- Buscando tarjetas conectadas ---")
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
    print(f"--- Obteniendo detalles del chip en {port} ---")
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
                print(f"  Chip: {info['chip']}")
                print(f"  MAC:  {info['mac']}")
                return info
    except:
        pass
    return None

def main():
    print("========================================")
    print("     Jmouse HzPro - Flash Tool")
    print("========================================\n")

    pio_cmd = check_pio()
    if not pio_cmd:
        sys.exit(1)

    detected_info = None
    boards = detect_boards(pio_cmd)
    if boards:
        print(f"Se encontraron {len(boards)} tarjeta(s):")
        for b in boards:
            print(f"  [+] {b['port']} - {b['description']}")
            detected_info = get_chip_info(b['port'])
    else:
        print("[!] No se detectaron tarjetas automaticamente.")

    environments = [
        {"id": "esp32dev", "name": "Solo Bluetooth (ESP32 Clasico)", "desc": "Para WROOM / WROVER. No soporta USB HID."},
        {"id": "esp32s3_ble", "name": "Dual: Bluetooth + USB (ESP32-S3)", "desc": "RECOMENDADO. Soporta ambos simultaneamente."},
        {"id": "esp32s3_usb", "name": "Solo USB Directo (ESP32-S3)", "desc": "Desactiva Bluetooth para maxima discrecion."},
        {"id": "esp32s2", "name": "Solo USB Directo (ESP32-S2)", "desc": "Para tarjetas S2. No tiene Bluetooth."},
    ]

    print("\nSelecciona tu configuracion:")
    for i, env in enumerate(environments, 1):
        print(f"{i}. {env['name']}")
        print(f"   - {env['desc']}")

    try:
        choice = input("\nElige una opcion (1-4) o 'q' para salir: ")
        if choice.lower() == 'q':
            sys.exit(0)
        
        idx = int(choice) - 1
        if 0 <= idx < len(environments):
            target = environments[idx]["id"]
            selected_env = environments[idx]
            print(f"\n>>> Iniciando programacion para: {selected_env['name']}...")
            
            # Compilar y subir
            cmd = f"{pio_cmd} run -e {target} -t upload"
            result = run_command(cmd)
            
            if result == 0:
                print("\n" + "="*40)
                print("       RESUMEN DE PROGRAMACION")
                print("="*40)
                print(f" ESTADO:    EXITOSO")
                print(f" MODO:      {selected_env['name']}")
                if detected_info:
                    print(f" CHIP:      {detected_info.get('chip', 'Desconocido')}")
                    print(f" MAC:       {detected_info.get('mac', 'Desconocido')}")
                print("-" * 40)
                print(" PROXIMOS PASOS:")
                print(" 1. Busca el WiFi: JmouseHzPro-Setup")
                print(" 2. Ve a: http://192.168.4.1")
                print(" 3. Configura tus patrones de movimiento")
                print("="*40)
                
                mon = input("\n¿Quieres abrir el Monitor Serie ahora? (s/n): ")
                if mon.lower() == 's':
                    print("\n--- Iniciando Monitor (115200 baudios) ---")
                    print("--- Presiona Ctrl+C para salir del monitor ---")
                    os.system(f"{pio_cmd} device monitor -b 115200")
            else:
                print("\n[!] Error durante la programacion.")
                print("Tips: Prueba manteniendo el boton BOOT presionado al conectar el USB.")
        else:
            print("Opción no válida.")
    except ValueError:
        print("Entrada no válida.")
    except KeyboardInterrupt:
        print("\nCancelado por el usuario.")

if __name__ == "__main__":
    main()
