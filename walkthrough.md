# Jmouse HzPro — Guía y Funcionalidades Premium

## ✅ Estado de Compilación (`esp32dev`)

```
RAM:   18.5% (60.5 KB / 320 KB)  --> ~260 KB Libres
Flash: 52.6% (1.65 MB / 3.00 MB) --> ~1.35 MB Libres
========================= [SUCCESS] =========================
```

---

## 📁 Estructura del Proyecto

| Archivo | Propósito |
|---------|-----------|
| [platformio.ini](file:///s:/MouseESP/platformio.ini) | 4 entornos de compilación (ESP32, S2, S3-BLE, S3-USB) con partición `huge_app.csv` |
| [config.h](file:///s:/MouseESP/src/config.h) | Configuración global de hardware y constantes |
| [usb_mouse.h](file:///s:/MouseESP/src/usb_mouse.h) / [.cpp](file:///s:/MouseESP/src/usb_mouse.cpp) | Driver USB HID nativo para ESP32-S2 y S3 |
| [storage.h](file:///s:/MouseESP/src/storage.h) / [.cpp](file:///s:/MouseESP/src/storage.cpp) | Persistencia NVS + **Autodiagnóstico y Auto-reparación** |
| [button_handler.h](file:///s:/MouseESP/src/button_handler.h) / [.cpp](file:///s:/MouseESP/src/button_handler.cpp) | Detección avanzada de clics y gestos de retención |
| [movement.h](file:///s:/MouseESP/src/movement.h) / [.cpp](file:///s:/MouseESP/src/movement.cpp) | Motor de 8 patrones de movimiento humanizados |
| [ble_manager.h](file:///s:/MouseESP/src/ble_manager.h) / [.cpp](file:///s:/MouseESP/src/ble_manager.cpp) | BLE Mouse + reconexión automática |
| [web_pages.h](file:///s:/MouseESP/src/web_pages.h) | Portal web embebido responsivo (HTML5/CSS3/JS) |
| [web_portal.h](file:///s:/MouseESP/src/web_portal.h) / [.cpp](file:///s:/MouseESP/src/web_portal.cpp) | Servidor WiFi AP/STA + Captive Portal + **mDNS (`jmouse.local`)** + **Auto-Sleep** |
| [main.cpp](file:///s:/MouseESP/src/main.cpp) | Orquestador principal y gestión de LED/Botón |

---

## 🎮 Controles del Botón BOOT (GPIO0)

| Gesto | Acción | Feedback LED |
|-------|--------|--------------|
| **Click simple** | Pausar / Reanudar jiggle | Cambia entre Fijo ON y Efecto Respiración |
| **Doble click** | Cambiar velocidad (5s, 15s, 30s, 60s) | Parpadea de 1 a 4 veces |
| **Triple click** | Reiniciar emparejamiento BLE + Forzar jiggle | Parpadeo rápido |
| **Mantener 3s** | Abrir/cerrar portal web WiFi | Al llegar a 3s, parpadea 3 veces rápido |
| **Mantener 10s** | Reseteo de Fábrica (Borrar NVS y Bluetooth) | A partir del seg 7 destella estilo "bomba" |
| **Sostener al conectar USB** | **🛟 Reseteo de Emergencia por Hardware** | Destello continuo de confirmación |

---

## 💡 Patrones LED Premium

| Estado | Comportamiento LED |
|--------|--------------------|
| **Desconectado** | Parpadeo lento (1s) indicando búsqueda |
| **Conectado + Activo** | LED fijo encendido (se apaga 10ms al hacer un movimiento) |
| **Conectado + Pausado** | **Efecto Respiración suave (PWM `ledc`)** |
| **Portal Web Activo** | Triple parpadeo distintivo |
| **Confirmación 3s** | 3 parpadeos rápidos indicando que se activará el portal |
| **Aviso Reseteo 10s** | Parpadeo muy rápido de advertencia |

---

## 🛟 "Salvavidas" y Tolerancia a Fallos (Novedades)

1. **🛟 Reseteo de Emergencia en Arranque (Hard Hardware Wipe):** Si alguna vez el dispositivo entra en bucle de reinicio o se corrompe el sistema, mantén presionado el botón BOOT *antes de conectar el cable USB* y sostenlo 3 segundos. El dispositivo formateará a bajo nivel la memoria NVS y los enlaces Bluetooth.
2. **🔌 Auto-Sleep del Portal Web (5 minutos):** Si activas el portal web WiFi pero olvidas cerrarlo, el dispositivo cerrará automáticamente el WiFi a los 5 minutos de inactividad para ahorrar energía y reanudar el jiggler.
3. **🌐 Acceso por Dominio Fácil (mDNS):** Puedes acceder al portal web escribiendo `http://jmouse.local` en tu navegador sin necesidad de recordar la IP `192.168.4.1`.
4. **🛡 Autodiagnóstico NVS:** Al arrancar, el sistema verifica la cordura de los datos guardados. Si la memoria está dañada, se autorrepara restaurando los valores de fábrica de forma transparente.
5. **🔄 Watchdog Bluetooth:** Si el ESP32 se queda desconectado del ordenador durante más de 3 minutos (por ejemplo, si el PC entró en reposo), reinicia automáticamente la publicidad BLE para facilitar una reconexión sin intervención del usuario.

---

## 🚀 Cómo Flashear

```bash
# Para ESP32 clásico (Bluetooth BLE)
pio run -e esp32dev -t upload

# Para ESP32-S3 usando Bluetooth BLE
pio run -e esp32s3_ble -t upload

# Para ESP32-S2 o S3 usando cable USB nativo
pio run -e esp32s2 -t upload
```
