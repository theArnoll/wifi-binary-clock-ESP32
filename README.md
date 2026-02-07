# A simple LED binary code on your desk

Useful: Probably 0  
Unique and fun: 100

Why did I waste my time making this?  
✨ Absolutely no reason ✨

Anyway

Function:

- Automatically get time from Wi-Fi.

- Display M/D H:M:S in binary

- Looks nice... probably

**THIS CODE IS UNTESTED.** Use it at your own risk. (Although there's probably no risk involved. I mean, how badly can a few LEDs go wrong?)

**Rename and modify `/BinaryClockESP32/WiFiConfig.h.template` before uploading code.** You should enter your Wi-Fi SSID and password into the file and rename the file to `WiFiConfig.h` (remove the `.template` extension) in order to compile and upload the file.

## Concept Sketch

![./conceptSketch.jpg](conceptSketch.jpg)

## BOM

| Part               | Quantity |
| ------------------ | -------- |
| ESP32-C3 SuperMini | 1        |
| LED                | 32 (8*4) |
| 74HC595            | 1        |
| 2x4 Decoder        | 1        |
| 1~1.1kΩ Resistor   | 8        |