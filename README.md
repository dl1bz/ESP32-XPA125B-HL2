---
# XPA125B band voltage and PTT with Hermes Lite 2
For automatic switch the band filter in XPA125B we need an analogue voltage for the PA depend from the selected band we want to use.
For this solution I use an ESP32 (TTGO LILYGO T-DisplayV1.1). I get via the internal UART at DB1 connector of HL2 (3.3V TTL, 9600,8,N,1) the current frequency of HL2.

![](https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/HL2_Connect_intern.jpg)

Every time you change the frequency, the HL2 sends a string via UART with format "FA00028500000;", here as example for the frequency 28500000Hz. This value I take, calculate the band, and set an analogue output with one of the DAC of ESP32. It's a PWM output, so I flatten the voltage with a resistor and a capacitor. In addition I use an optocoupler 4N25 for an isolated PTT switch for the PA. Look at the https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/ESP32-XPA125B-HL2-Schematic.pdf for details.

As goodie I use the 1.14" Colordisplay of the T-Display for showing frequency and band.
![](https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/Prototypeboard_ESP32_HL2.jpg)

## Source code
The code https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/XPA125B-HL2.ino is written with the Arduino IDE and the ESP32 extensions. Look around, how need the Arduino IDE to setup for use with ESP32. Additional we need the Library for the TFT-display, you can find here https://github.com/Xinyuan-LilyGO/TTGO-T-Display/tree/master/TFT_eSPI (required for compilation).

## Good to know
As a special feature I use multitasking with the ESP32. One task (band voltage) runs at core 1 and the second task (PTT switch) runs at core 0, so both tasks work independent.

## Final words for note ##
This program is free software only for personal use; you can redistribute it and/or modify it how you want.
The codebase is "as is" without any kind of support of me.
It's NOT for commercial use in any case.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

73 Heiko, DL1BZ
