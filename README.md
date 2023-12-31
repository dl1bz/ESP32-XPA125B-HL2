---
# XPA125B band voltage and PTT with Hermes-Lite 2

## Preamble
The goal was to build a very simple, cost-effective interface for automatic bandswitching and isolated PTT switch of my XPA125B. SMD is not for everyone, so I want to use "classic components" for this.
The scope should be as low as possible and with only small and simple modifications of HL2 itself. 

For automatic switch the band filter in XPA125B we need an analogue voltage for the PA depend from the selected band we want to use.
For this solution I use an ESP32 (TTGO LILYGO T-Display V1.1, cost ~20€). I get via the internal UART at DB1 connector of HL2 (3.3V TTL, 9600,8,N,1) the current frequency of HL2.

![](https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/HL2_Connect_intern.jpg)

Every time you change the frequency, the HL2 sends a string via UART with format "FA00028500000;", here as example for the frequency 28500000Hz. This value I take, calculate the band, and set an analogue output with one of the DAC of ESP32. It's a PWM output, so I flatten the voltage with a resistor and a capacitor. In addition I use an optocoupler 4N25 for an isolated PTT switch for the PA. Look at the https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/ESP32-XPA125B-HL2-Schematic.pdf for details.

As goodie I use the 1.14" Colordisplay of the T-Display for showing frequency and band.
![](https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/Prototypeboard_ESP32_HL2.jpg)

## Connect the XPA125B
For a connection with the XPA125B I installed a SUBD-9 socket (rear of HL2). Look at the manual of XPA125B for the pinout of external PA socket (called ACC rear of XPA125B, 6 pole Mini-DIN female). The cables between HL2, interface and PA I build by myself.

## Power supply
As power supply I use a 5V regulator with a 7805, so the interface board can be powered with 13.8V. It's possible - as option - to use +3.3V from HL2.

## Source code
I'm not a professional programmer, it's only a hobby for me. The code is not perfect and can be surely optimized. If you think so, do it by yourself.

The code https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/XPA125B-HL2.ino is written with the Arduino IDE and the ESP32 extensions. Look around, how need the Arduino IDE to setup for use with ESP32. Additional we need the Library for the TFT-display, you can find here https://github.com/Xinyuan-LilyGO/TTGO-T-Display/tree/master/TFT_eSPI (required for compilation). Install it in your Arduino IDE environment.

## Good to know
- first power up the interface before starts the SDR application like Thetis. That's required, because if the HL2 will be initialized he sends his frequency via UART and the PA can select the correct band. Otherwise you need to change the frequency one time, then the HL2 sends the frequency too via UART (or better, the HL2 sends every frequency change via UART always)
- as a special feature I use multitasking with the ESP32. One task (band voltage) runs at core 1 and the second task (PTT switch) runs at core 0, so both tasks work independent.
- you can use this interface with the Micro PA50 too, if you want to use the PA50 in manual mode with PTT switch and not in automatic mode with RF-VOX. With PTT switch the automatic band select of PA50 don't work ! You need to select in setup of the PA50 "XIEGU" as band voltage protocol. It's the same as for the XPA125B.

##  Final words for note ##
This program is free software only for personal use; you can redistribute it and/or modify it how you want.
The codebase is "as is" without any kind of support of me.
It's NOT for commercial use in any case.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

73 Heiko, DL1BZ
