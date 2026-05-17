import os
import shutil
import subprocess
import sys

def check_pio():
    result = subprocess.run("pio --version", shell=True, capture_output=True)
    if result.returncode == 0: return "pio"
    result = subprocess.run(f"{sys.executable} -m platformio --version", shell=True, capture_output=True)
    if result.returncode == 0: return f"{sys.executable} -m platformio"
    return None

def main():
    print("==================================================")
    print("   Jmouse HzPro - Exportador para Web Flasher")
    print("==================================================\n")
    print("Generando binarios limpios para https://esptool.spacehuhn.com/ ...\n")

    pio_cmd = check_pio()
    if not pio_cmd:
        print("[!] Error: PlatformIO no está instalado o no está en el PATH.")
        sys.exit(1)

    environments = [
        {"id": "esp32dev", "name": "ESP32_Clasico_Bluetooth", "boot_offset": "0x1000"},
        {"id": "esp32s3_ble", "name": "ESP32_S3_Dual_Bluetooth_USB", "boot_offset": "0x0"},
        {"id": "esp32s3_usb", "name": "ESP32_S3_Solo_USB", "boot_offset": "0x0"},
        {"id": "esp32s2", "name": "ESP32_S2_Solo_USB", "boot_offset": "0x0"},
    ]

    base_export_dir = "release_binaries"
    os.makedirs(base_export_dir, exist_ok=True)

    summary_text = """============================================================
GUIA DE PROGRAMACION POR NAVEGADOR (WEB FLASHER)
============================================================

Puedes programar tu ESP32 desde Google Chrome o Microsoft Edge
sin instalar Python, PlatformIO ni ninguna herramienta.

1. Conecta tu ESP32 por USB a tu computadora.
2. Abre tu navegador e ingresa a: https://esptool.spacehuhn.com/
3. Haz clic en "Connect" y selecciona el puerto COM de tu ESP32.
4. Selecciona tu modelo en las carpetas de este directorio y carga los 3 archivos con las siguientes direcciones de memoria exactas (Offsets):

------------------------------------------------------------
A) PARA ESP32 CLASICO (ESP32_Clasico_Bluetooth):
------------------------------------------------------------
[ 0x1000  ] -> bootloader.bin
[ 0x8000  ] -> partitions.bin
[ 0x10000 ] -> firmware.bin

------------------------------------------------------------
B) PARA ESP32-S3 y ESP32-S2 (Todos los modelos S2 y S3):
------------------------------------------------------------
[ 0x0     ] -> bootloader.bin
[ 0x8000  ] -> partitions.bin
[ 0x10000 ] -> firmware.bin

------------------------------------------------------------
5. Haz clic en el botón "Program" (o "Flash").
6. ¡Listo! Cuando termine, presiona el botón EN/RST de tu placa.

"""

    with open(os.path.join(base_export_dir, "INSTRUCCIONES_WEB_FLASHER.txt"), "w", encoding="utf-8") as f:
        f.write(summary_text)

    for env in environments:
        env_id = env["id"]
        folder_name = env["name"]
        boot_offset = env["boot_offset"]
        
        print(f">>> Compilando entorno: {env_id} ({folder_name})...")
        cmd = f"{pio_cmd} run -e {env_id}"
        result = subprocess.run(cmd, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        
        if result.returncode == 0:
            build_dir = os.path.join(".pio", "build", env_id)
            target_dir = os.path.join(base_export_dir, folder_name)
            os.makedirs(target_dir, exist_ok=True)
            
            files_to_copy = ["bootloader.bin", "partitions.bin", "firmware.bin"]
            success = True
            for file in files_to_copy:
                src = os.path.join(build_dir, file)
                dst = os.path.join(target_dir, file)
                if os.path.exists(src):
                    shutil.copy2(src, dst)
                else:
                    success = False
            
            if success:
                # Escribir un pequeño archivo info.txt en la carpeta
                with open(os.path.join(target_dir, "offsets.txt"), "w", encoding="utf-8") as f:
                    f.write(f"Offsets para {folder_name} en esptool.spacehuhn.com:\n")
                    f.write(f"{boot_offset}  -> bootloader.bin\n")
                    f.write(f"0x8000  -> partitions.bin\n")
                    f.write(f"0x10000 -> firmware.bin\n")
                print(f"  [+] Binarios exportados correctamente en: {target_dir}")
            else:
                print(f"  [!] Advertencia: No se encontraron todos los binarios para {env_id}.")
        else:
            print(f"  [!] Error al compilar {env_id}.")

    print("\n" + "="*50)
    print(f" PROCESO COMPLETADO. Revisa la carpeta: {base_export_dir}/")
    print("="*50)

if __name__ == "__main__":
    main()
