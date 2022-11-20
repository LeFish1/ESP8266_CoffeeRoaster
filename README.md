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
                                                 3V3-++   V~+-230V~L
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

(https://asciiflow.com/#/share/eJzFU8FKw0AQ%2FZVlrmmgJiTR3KQVA01rMbB42EsoewimEUqEllK%2FQvox4tf4Ja5ujTtx0qTx0LCH2d3Z9968mWyhSJcSwuI5zweQpxu5ghC2AtYCwqvAHQjYqMgJAhWVcl2qjQDW70uSeyGKXk8%2FXt%2FJ1QfP5a5%2BbBnQjjvkL%2FE%2F5FXBGTCa3NHLpnJOZ2gjqymPMjvOikcWxRN7Ph0itJtkfun4PsYl2CKZlnJlSj1eaR2tJfUYeSuRUvU9MyxmJrPFnw7D5XF94HF9wLOiwm9kJksxJrTmBRmj%2FfgC4U2vH3w%2F8MhU9mf0Ghis6g9SAbrs2pvu7s6Qu%2FaPu7je32o9Ei0ZTRjji8WhM0r1aTPVwIbqbrAQOb1%2FI0X7JOsoaQVE4H1VIQi9bmfjL10BbeadulMZZq86aOswGu1d6EC1p%2F4s41bADnaf3TuEEg%3D%3D)

# Build and Test
Enter your WiFi credentials in sketch.
Enter the IP address of the ESP8266 matching your subnet.
Build with platformio and transfer to ESP8266.

# Artisan settings
Make sure the artisan settings match the pictures in this repo [/artisan](https://github.com/LeFish1/ESP8266_CoffeeRoaster/tree/master/artisan)

# Have fun roasting!
