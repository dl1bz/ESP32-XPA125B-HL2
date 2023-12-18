---
# XPA125B band voltage and PTT from Hermes Lite 2
For automatic switch the band filter in XPA125B we need an analogue voltage for the PA depend from the selected band we want to use.
For this solution I use an ESP32 (TTGO LILYGO T-DisplayV1.1). I get via the internal UART at DB1 connector of HL2 (3.3V TTL, 9600,8,N,1) the current frequency of HL2. Every time you change the frequency, the HL2 sends a string in format "FA00028500000;", here as example for the frequency 28500000Hz. This value I take, calculate the band, and set an analogue output with one of the DAC of ESP32. It's a PWM output, so I flatten the voltage with a resistor and a capacitor. In addition I use an optocoupler 4N25 for an isolated PTT switch for the PA. Look at the https://github.com/dl1bz/ESP32-XPA125B-HL2/blob/main/ESP32-XPA125B-HL2-Schematic.pdf for details.

As goodie I use the 1.14" Colordisplay of the T-Display for showing frequency and band.
