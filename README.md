# 🖱️ Jmouse HzPro (MouseESP) — Advanced ESP32 Mouse Jiggler

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg?style=for-the-badge)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Supported-orange?style=for-the-badge&logo=platformio)
![ESP32](https://img.shields.io/badge/ESP32%20%2F%20S2%20%2F%20S3-Dual%20Mode%20%28BLE%20%26%20USB%29-success?style=for-the-badge&logo=espressif)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge)

**Jmouse HzPro** es un firmware de código abierto y grado profesional que transforma cualquier microcontrolador de la familia ESP32 en un emulador de movimiento de ratón (Mouse Jiggler) altamente avanzado e indetectable. Diseñado para operar tanto de forma inalámbrica por **Bluetooth Low Energy (BLE)** como por cable mediante **USB HID Nativo**, es la solución definitiva para evitar bloqueos de sesión y sistemas de monitoreo de inactividad.

---

## ✨ Características Principales

* 🛡️ **100% Indetectable (Simulación Humana)**: Utiliza algoritmos estocásticos que varían aleatoriamente los tiempos y ejecutan trayectorias curvas naturales (Bézier), incluyendo pausas largas simuladas (5-15 min) para evadir herramientas de análisis heurístico.
* 🌐 **Portal Web Embebido (`http://jmouse.local`)**: Configura todo en tiempo real desde tu móvil o PC mediante una interfaz web asíncrona y moderna (Modo Oscuro, responsive HTML5/JS) sin necesidad de instalar software en el ordenador ni reprogramar el chip.
* 🔌 **Auto-Sleep y Ahorro Energético**: El portal web apaga automáticamente el WiFi tras 5 minutos de inactividad para garantizar que el chip dedique toda su memoria y rendimiento a la estabilidad del emulador de ratón.
* 🎮 **Control Físico Todo-en-Uno**: Un solo botón físico (`GPIO0`) permite pausar/reanudar, cambiar velocidades, activar el WiFi o realizar un reseteo completo de fábrica.
* 💡 **Efecto Visual Premium**: Retroalimentación LED intuitiva con efecto *respiración suave (breathing LED)* mediante PWM por hardware mientras está pausado, y parpadeos distintivos para confirmaciones.
* 🛟 **Tolerancia a Fallos Robusta**: Autodiagnóstico de memoria NVS con auto-reparación en el arranque, Watchdog de reconexión BLE y borrado de emergencia a bajo nivel por hardware.

---

## 🛠️ Compatibilidad de Placas y Hardware

Jmouse HzPro detecta y se adapta a la arquitectura de tu ESP32 automáticamente según el entorno de compilación seleccionado:

| Modelo de ESP32 | Conexión con PC | Entorno PlatformIO | Recomendado para |
| :--- | :--- | :--- | :--- |
| **ESP32 Clásico** | Inalámbrica (Bluetooth BLE) | `esp32dev` | Placas estándar (DevKit V1, NodeMCU-32S). |
| **ESP32-S2** | Cableada (USB HID Nativo) | `esp32s2` | Placas S2 con puerto USB OTG. Sin uso de Bluetooth. |
| **ESP32-S3** | Inalámbrica (Bluetooth BLE) | `esp32s3_ble` | **Recomendado:** Máximo rendimiento con procesador dual-core. |
| **ESP32-S3** | Cableada (USB HID Nativo) | `esp32s3_usb` | Conexión directa y ultra rápida por cable USB nativo. |

---

## 🚀 Guía Rápida de Instalación

Hemos preparado este repositorio para que cualquier usuario, con o sin experiencia en programación, pueda poner a funcionar su dispositivo en pocos minutos.

### Opción A: Instalación Automatizada (Recomendada)
No necesitas recordar comandos complejos. El repositorio incluye un asistente interactivo en Python:

1. **Requisitos previos**: Asegúrate de tener instalado [Python 3](https://www.python.org/downloads/) (marca la casilla *"Add Python to PATH"* durante la instalación) y [PlatformIO Core CLI](https://docs.platformio.org/en/latest/core/index.html) (`pip install -U platformio`).
2. **Controladores USB-Serial (Drivers)**: Si tu ordenador no detecta el ESP32 al conectarlo, instala el driver correspondiente a tu placa:
   * **Placas con chip CP210x**: [Descargar Drivers de Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
   * **Placas con chip CH340 / CH34x**: [Descargar Drivers de WCH](http://www.wch-ic.com/downloads/CH341SER_EXE.html)
3. Conecta tu ESP32 al ordenador mediante un cable USB (asegúrate de que sea un cable de datos y no solo de carga).
3. Abre una terminal en la carpeta del proyecto y ejecuta el asistente:
   ```bash
   python flash_tool.py
   ```
4. Sigue las sencillas instrucciones del menú en pantalla para elegir tu placa y el puerto de conexión. ¡El script compilará y flasheará tu dispositivo automáticamente!

### Opción B: Instalación Manual vía CLI o VSCode
Si utilizas VSCode con la extensión de PlatformIO o prefieres la consola:
```bash
# Para ESP32 estándar por Bluetooth:
pio run -e esp32dev -t upload

# Para ESP32-S3 por Bluetooth (Recomendado):
pio run -e esp32s3_ble -t upload

# Para ESP32-S2 / S3 mediante cable USB HID:
pio run -e esp32s2 -t upload
```

### Opción C: Flasheo por Navegador Web (Sin instalar herramientas)
Si no deseas instalar Python ni PlatformIO en tu computadora, puedes usar Google Chrome o Microsoft Edge para cargar directamente los binarios precompilados:
1. Conecta tu placa ESP32 por USB y abre [ESPTool Web Flasher (Spacehuhn)](https://esptool.spacehuhn.com/).
2. Haz clic en **"Connect"** y selecciona el puerto COM de tu placa.
3. Ve a la carpeta `release_binaries/` de este repositorio (puedes generarlos ejecutando `python export_web_binaries.py`).
4. Selecciona tu modelo y añade los 4 archivos binarios con las siguientes direcciones exactas (offsets):
   * **Para ESP32 Clásico:**
     * `0x1000` -> `bootloader.bin`
     * `0x8000` -> `partitions.bin`
     * `0xe000` -> `boot_app0.bin`
     * `0x10000` -> `firmware.bin`
   * **Para ESP32-S2 y ESP32-S3:**
     * `0x0` -> `bootloader.bin`
     * `0x8000` -> `partitions.bin`
     * `0xe000` -> `boot_app0.bin`
     * `0x10000` -> `firmware.bin`
5. Haz clic en **"Program"** (o Flash) y al finalizar presiona el botón EN/RST de tu placa para iniciar.

---

## 🎮 ¿Cómo se usa? (Controles Físicos y LED)

Al conectar el dispositivo por primera vez a un cargador o al PC:

### 1. Conexión Inicial
* **En modo Bluetooth (BLE)**: Busca en tu ordenador un nuevo dispositivo Bluetooth llamado **"Jmouse HzPro"** y conéctalo. Mientras busca, el LED parpadeará lentamente.
* **En modo USB HID**: Al conectar el cable al puerto USB nativo, el PC lo reconocerá instantáneamente como un ratón físico estándar.

### 2. Uso del Botón BOOT (`GPIO0`)
| Acción en el Botón | Función Ejecutada | Reacción del LED |
| :--- | :--- | :--- |
| **1 Clic Corto** | **Pausar / Reanudar**: Detiene o activa el movimiento. | Pausado: *Efecto Respiración suave*. Activo: *Luz fija*. |
| **2 Clics Rápidos** | **Velocidad**: Alterna el intervalo (5s, 15s, 30s, 60s). | Parpadea de 1 a 4 veces confirmando el nivel. |
| **3 Clics Rápidos** | **Forzar Jiggle / BLE**: Resetea el Bluetooth y fuerza movimiento. | Parpadeo ultrarrápido de confirmación. |
| **Mantener 3 segundos** | **Portal Web WiFi**: Activa/Desactiva la red de configuración. | Al llegar a los 3s, destella 3 veces rápidamente. |
| **Mantener 10 segundos**| **Reset de Fábrica**: Borra configuraciones y emparejamientos.| Al segundo 7, parpadeo de advertencia estilo "bomba". |
| **Mantener al conectar USB**| **Wipe de Emergencia**: Formateo físico de bajo nivel. | Destello continuo de confirmación en el arranque. |

---

## 🌐 Configuración desde el Portal Web (WiFi / mDNS)

Para personalizar al máximo los patrones de movimiento sin necesidad de tocar código ni cables:

1. Mantén pulsado el botón BOOT durante **3 segundos** hasta ver 3 parpadeos rápidos en el LED.
2. Desde tu teléfono móvil, tablet o PC, conéctate a la red WiFi abierta:
   * 📶 **Nombre de Red (SSID)**: `JmouseHzPro-Setup`
3. Abre tu navegador de internet y entra a la dirección web:
   * 🔗 `http://jmouse.local` (o introduce la IP `192.168.4.1`).
4. Desde este panel de control intuitivo podrás:
   * Alternar entre los **8 modos de movimiento** (Bézier, Círculos, Aleatorio, Micro Jiggle 1px, etc.).
   * Ajustar la amplitud del movimiento y los porcentajes de aleatoriedad.
   * Gestionar y eliminar los emparejamientos de dispositivos guardados en las ranuras de memoria.

> [!NOTE]  
> **Ahorro de Batería y RAM (Auto-Sleep):** Si dejas el portal web abierto y dejas de usarlo, el ESP32 apagará la red WiFi automáticamente a los **5 minutos** para evitar consumo innecesario y garantizar un rendimiento estable e ininterrumpido.

---

## 📚 Documentación Técnica Profesional

Si deseas explorar a fondo la arquitectura interna del sistema, diagramas de flujo de estado, formulación matemática de los algoritmos anti-detección estocástica y la guía de desarrollo del repositorio, consulta nuestro manual completo:

👉 **[Ver Manual Técnico y Documentación Profesional (DOCUMENTACION.md)](DOCUMENTACION.md)**

---

## 🤝 Contribuciones y Soporte
¡Las contribuciones son muy bienvenidas! Siéntete libre de abrir un *Issue* para reportar sugerencias o enviar un *Pull Request* para añadir nuevas funciones. Si este proyecto te ha resultado útil, no olvides darle una ⭐ en GitHub.

<div align="center">
  <b>Hecho con ❤️ para la comunidad de código abierto e ingenieros de hardware.</b>
</div>
