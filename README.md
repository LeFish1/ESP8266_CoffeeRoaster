# Introduction 
This is an implementation of BilloJoeV2 (https://www.kaffee-netz.de/threads/umbau-heissluftfritteuse-zu-artisan-gesteuertem-kaffeeroester-billojoe-v2-komplettes-tutorial.133648/) on an ESP8266 with Websocket-Interface to Artisan (https://artisan-scope.org/devices/websockets/)

# Getting Started
You need VSCode and platformio.
Import this project, build and transfer to ESP8266.

# Circuit

A basic circuit could be the following:
```

                                                       SSR
                                                     +------+
                                                 +5V-++   V~+-230V~L
                                                     |      |
                                                     |      |
                                            +--------+-   V~+---+
                                            |        +------+   |
      Hi-Link HLK-PM0             ESP8266   |                   |     Heater
         +---------+             +--------+ |                   |   +---------+
230V~ L -+      +Vo+-+5V     +5V-+Vin     | |                   +---+         +-230V~ N
         |         |             |      D1+-+    MAX6675            |         |
         |         |        +3V3-+3V3     |     +--------+          +---------+
230V~ N -+      -Vo+-+           |      D5+-----+SCK  Vcc+-+3V3
         +---------+ |           |        |     |        |
                     v           |      D6+-----+CS      |
                                 |        |     |        |
                               +-+GND   D7+-----+SO   GND+-+
                               | +--------+     +--------+ |
                               v                           v
```

(https://asciiflow.com/#/share/eJzFU8FqwkAQ%2FZVlrjFgE7JpcytaGjBa6cLSw16C5BBMU5AUFLFfUfyY0q%2Fpl3TbqN2JExPTg2EPs9nZ9968mV1DHj8nEOSvWdaDLF4lCwhgrWCpILjx3Z6ClY4c39dRkSwLvVHAun1CPCqVd7r69f5Jri54lifLy5YB7bh9%2BRb9Q94huABGnTvlsqmc8xmayCrKw9SO0nzOwmhkT8d9hHYnptcO5xiXYAuTuEgWptTTlVbRGlJPkTcSaVW%2FM8MiZjJb8mU3XJ4sf%2BynTab5Ab%2BWmSzFmNCKF2SM9sMrhDe%2BfeLc98hUdjR6NQyWK90SUgfosG1v2rs7Qe7ae3dxvX%2FVeiSaGIwYk7PZrjNa9XkzVcOG6q6xEDm9%2FSBFc5J1IBoBEXhXVQiiXPeT4Y8unzbzQZ%2FpDLNXLbS1GI3mLrSg2lIvyzhVsIHNN7kEhAw%3D)

# Build and Test
Enter your WiFi credentials in sketch.
Enter the IP address of the ESP8266 matching your subnet.
Build with platformio and transfer to ESP8266.

# Artisan settings
Make sure the artisan settings match the pictures in this repo (/artisan)

# Have fun roasting!
