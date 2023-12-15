#define VERSION "XPA125B control for HL2 V1.0 by DL1BZ, 2023"

// Generate band voltage for a XPA125B from the UART output of a Hermes-Lite 2
// using a LILYGO T-DISPLAY ESP23 with 1.14" LCD Display integrated

// ATTENTION: you need RxD from HL2 connect to TxD of ESP32 and TxD of HL2 connect to RxD of ESP32
// PIN 32 : TxD UART2 (from/to HL2 9600,8,N,1)
// PIN 33 : RxD UART2 (from/to HL2 9600,8,N,1)

// PIN 27 : output analog voltage to XPA125B with R=470Ohm and C=22uF for flatten the PWM output
// PIN 26 : PTT output HIGH ACTIVE to opto-coupler 4N25 for TX switching the XPA125B on/off
// PIN 25 : PTT input LOW ACTIVE from EXTTR signal of HL2 with resistor 10kOhm as PULL_UP between +3.3V and PIN 25 from the ESP32

// the ESP32 is connected via Step-Down voltage converter +5V output and +13.8V input

// I'm not a professional programmer, so the coding is not perfect but it works :-) 

#include <TFT_eSPI.h>       //using this LIB https://github.com/Xinyuan-LilyGO/TTGO-T-Display/tree/master/TFT_eSPI  
// IMPORTANT!  
//      In the "User_Setup_Select.h" file, enable "#include <User_Setups/Setup25_TTGO_T_Display.h>"
//-----------------------------------------------------------------------------------------
//for TFT
TFT_eSPI tft = TFT_eSPI();

#define screen_width  240       //placement of text etc must fit withing these boundaries.
#define screen_heigth 135

// all my known colors for ST7789 TFT (but not all used in program)
#define B_DD6USB 0x0004    //   0,   0,   4
#define BLACK 0x0000       //   0,   0,   0
#define NAVY 0x000F        //   0,   0, 123
#define DARKGREEN 0x03E0   //   0, 125,   0
#define DARKCYAN 0x03EF    //   0, 125, 123
#define MAROON 0x7800      // 123,   0,   0
#define PURPLE 0x780F      // 123,   0, 123
#define OLIVE 0x7BE0       // 123, 125,   0
#define LIGHTGREY 0xC618   // 198, 195, 198
#define DARKGREY 0x7BEF    // 123, 125, 123
#define BLUE 0x001F        //   0,   0, 255
#define GREEN 0x07E0       //   0, 255,   0
#define CYAN 0x07FF        //   0, 255, 255
#define RED 0xF800         // 255,   0,   0
#define MAGENTA 0xF81F     // 255,   0, 255
#define YELLOW 0xFFE0      // 255, 255,   0
#define WHITE 0xFFFF       // 255, 255, 255
#define ORANGE 0xFD20      // 255, 165,   0
#define GREENYELLOW 0xAFE5 // 173, 255,  41
#define PINK 0xFC18        // 255, 130, 198
//*************************************************************

//=================================================
// Mapping of port-pins to functions on ESP32
//=================================================

// the Pins for SPI
#define TFT_CS    5
#define TFT_DC   16
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_RST  23
#define TFT_BL    4

#define NUM_BANDS 11                // Number of Bands (depending on the radio)
#define PTTpin    26                // PTT out pin

boolean LastState = true;

//=========================================================================================
uint8_t G_currentBand = NUM_BANDS;  // Band in use (default: not defined)

//-----------------------------------------------------------------------------------------
// creating bandinfo based on the frequency info
//-----------------------------------------------------------------------------------------
// tables for band selection and bittpattern calculation
// !!! pls adapt "NUM_BANDS" if changing the number of entries in the tables below !!!
// lower limits[kHz] of the bands
constexpr unsigned long lowlimits[NUM_BANDS] = {
  1000, 2751, 4501,  6001,  8501, 13001, 16001, 19501, 23001, 26001, 38001
};
// upper limits[kHz] of the bands
constexpr unsigned long uplimits[NUM_BANDS] = {
  2750, 4500, 6000,  8500, 13000, 16000, 19500, 23000, 26000, 38000, 60000
};

// "xxM" display for the TFT display. ie show what band the unit is current on in "meters"
const String (band2string[NUM_BANDS + 1]) = {
    // 160     80     60     40     30     20     17     15     12     10      6     NDEF
      "160m"," 80m"," 60m"," 40m"," 30m"," 20m"," 17m"," 15m"," 12m"," 10m","  6m"," Out"

};

//-----------------------------------------------------------------------------------------
// get the bandnumber matching to the frequency (in kHz)
//-----------------------------------------------------------------------------------------
byte get_Band(unsigned long frq) {
    byte i;
    for (i = 0; i < NUM_BANDS; i++) {
        //for (i=1; i<NUM_BANDS; i++) {   
        if ((frq >= lowlimits[i]) && (frq <= uplimits[i])) {
            return i;
        }
    }
    return NUM_BANDS; // no valid band found -> return not defined
}

//------------------------------------------------------------------
//    Show band in 'Meters' text on TFT
//------------------------------------------------------------------
void show_Band_TFT(void)
{
    // Show Freq[KHz]
    tft.setCursor(5, 120); 
    tft.fillRect(0, 80, 105, 55, BLACK); 
    tft.drawRoundRect(0, 80, 105, 55, 5, WHITE);
    tft.setTextColor(YELLOW);
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setTextSize(2);
    tft.print(band2string[G_currentBand]);
}

void Clear_Scr() {
        tft.fillRect(0, 31, 240, 104, BLACK);
}

//-----------------------------------------------------------------------------------------
// initialise the TFT display
//-----------------------------------------------------------------------------------------

void init_TFT(void)
{
    //tft.init(screen_heigth, screen_width) ;  //not used

    tft.init();
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);              // switch backlight on

    tft.fillScreen(BLACK);
    tft.setRotation(1);
    tft.fillRoundRect(0, 0, tft.width(), 30, 5, MAROON);   // background for screen title
    tft.drawRoundRect(0, 0, tft.width(), 30, 5, WHITE);    //with white border.

    tft.setFreeFont(NULL);               // Set default font
    tft.setTextSize(2);                  //for default Font only.Font is later changed.
    tft.setTextColor(YELLOW);
    tft.setCursor(20, 7);                //top line
    tft.print("HL2 <-> XPA125B");

    tft.setTextColor(WHITE);            //white from now on
}

void Draw_TX_TFT() {
        tft.fillRoundRect(190, 80, 50, 55, 10, RED);
        tft.drawLine(215, 88, 215, 128, WHITE);
        tft.drawLine(194, 88, 215, 103, WHITE);
        tft.drawLine(235, 88, 215, 103, WHITE);
        // tft.print("Tx");
}

void Draw_RX_TFT() {
        tft.fillRoundRect(190, 80, 50, 55, 10, GREEN);
        tft.fillRect(197, 90, 15, 30, BLACK);
        tft.drawLine(232, 85, 232, 125, BLACK);
        tft.drawLine(232, 85, 212, 90, BLACK);
        tft.drawLine(232, 125, 212, 120, BLACK);
        // tft.print("Rx");
}

int bandvoltage;
#define LED 27       //Band voltage
// For analogue PWM output
int brightness = 0;
const int freq = 5000;     //PWM Freq
const int ledChannel = 0;  //
const int resolution = 10; // possible resolution 8, 10, 12, 15 bit

// define external UART at PIN 33/RxD and PIN 32/TxD, 3.3V UART Level !!!
#define RXD2 33 // RxD ESP32 <-> TxD HL2
#define TXD2 32 // TxD ESP32 <-> RxD HL2

void init_DAC() {
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LED, ledChannel);
}

void init_UARTs() {
  Serial.begin(115200);                         // initialize internal UART with 115200/8N1
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // initialize external UART with 9600/8N1
}

void init_GPIO() {
  // pinMode(25, INPUT_PULLUP); // replaced with resistor 10kOhm at the ESP32 board against +3.3V, otherweise no correct detection if PA is disconnected
  pinMode(25, INPUT);           // PIN 25 input from the EXTTR line of HL2, LOW ACTIVE
  pinMode(26, OUTPUT);          // PIN 26 output to the 4N25 opto-coupler at the ESP32 board, HIGH ACTIVE
}

//------------------------------------------------------------
// process the frequency received from the radio
//------------------------------------------------------------

void showQRG_TFT(unsigned long frequency) {
    unsigned long freq_kHz;
    
    freq_kHz = frequency / 1000;            // frequency is now in kHz
    tft.setFreeFont(&Orbitron_Light_32);
    tft.setTextSize(1);
    tft.setCursor(5, 67);
    
    if (freq_kHz < 100000) {
      tft.setCursor(25, 67);                 // for bigger print size
    } 
    if (freq_kHz < 10000) {
       tft.setCursor(40, 67);
    }

    tft.fillRoundRect(0, 35, tft.width(), 40, 5, BLUE);
    // tft.drawRoundRect(0, 35, tft.width(), 40, 5, BLACK);    //with white border.
    tft.setTextColor(WHITE);
    tft.print(0.000001 * frequency, 5);    //show Frequency in MHz with 5 decimal digits

    // Test-output to serial monitor:
    // Serial.print("Frequency: ");  Serial.println(freq_kHz);
    // Serial.print("Band: ");     Serial.println(G_currentBand);
    // Serial.println(band2string[G_currentBand]);
}

// define two task handler for using multitasking with ESP32
TaskHandle_t PTTTask;
TaskHandle_t VoltageTask;

// Init hardware
void setup() {
// put your setup code here, to run once:

init_TFT();   // initialize T-DISPLAY LILYGO TTGO v1.1
init_DAC();   // initialize analog output
init_UARTs(); // initialize UARTs
init_GPIO();  // initialize GPIO 25 and 26
// Draw_RX_TFT();    // show RX symbol at TFT

Serial.println(VERSION);

// debug console output
Serial.println("Serial0 TXD is on pin: "+String(TX));
Serial.println("Serial0 RXD is on pin: "+String(RX));
Serial.println("Serial2 TXD is on pin: "+String(TXD2));
Serial.println("Serial2 RXD is on pin: "+String(RXD2));


// we use Multitasking with the ESP32
// Task 1 PTT loop
xTaskCreatePinnedToCore(
      PTTLoop, /* Function to implement the task */
      "PTTTask",   /* Name of the task */
      10000,     /* Stack size in words */
      NULL,      /* Task input parameter */
      1,         /* Priority of the task */
      &PTTTask,    /* Task handle. */
      0);        /* Core where the task should run */

// Task 2 Voltage loop
xTaskCreatePinnedToCore(
      VoltageLoop, /* Function to implement the task */
      "VoltageTask",   /* Name of the task */
      10000,     /* Stack size in words */
      NULL,      /* Task input parameter */
      0,         /* Priority of the task */
      &VoltageTask,    /* Task handle. */
      1);        /* Core where the task should run */

}

// main program loop
void loop() {
// here we do nothing, because we use two other tasks
delay(10); // wait 10ms
}

// Task 1 Voltage loop at Core 1
void VoltageLoop( void * pvParameters) {
String lastUARTdata;
String rawUARTdata;

// start program loop
for(;;) {

  // check if external UART is active for receive data from HL2
  while (Serial2.available()) {
  
  rawUARTdata = Serial2.readStringUntil(';');     // read data from HL2 UART sent in format FA00028500000;

  // check if UART data start with FA
  if (rawUARTdata.startsWith("FA", 0)) {
    String QRG_Hz = rawUARTdata.substring(5);            // cutoff first 5 characters from UART data (FA000 or FB000) because we have VFO A and B at HL2
    long Freq_Hz = QRG_Hz.toInt();                       // transform QRG string to numeric
    showQRG_TFT(Freq_Hz);                                // show Frequency at TFT
    Serial.print(Freq_Hz*0.001); Serial.println(" kHz"); // print Frequency
    
    G_currentBand = get_Band(0.001*Freq_Hz);             // get band as numeric
    // Serial.println(G_currentBand);                    // show band as numeric
    Serial.println(band2string[G_currentBand]);          // output as XXm (80m etc)
    
    // Set the bandvoltage
    int bandcode;
    bandcode = G_currentBand;
        int sendDAC;  // calculated value we will send to DAC at the end
        int corrFact; // because if a load exist like PA we will have a lower, non-linear voltage output as calculated
        corrFact = 0; // add a little bit more mV, here with R=470Ohm and C=22uF at PIN 27

    switch (bandcode) {
    case 0:  // 160M
        // Manual XPA125B 230mV
        bandvoltage=230; // in mV
        // 3
        corrFact = 6;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 1:  // 80M
        // Manual XPA125B 460mV
        bandvoltage = 460;
        // 4
        corrFact = 14;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 2:  // 60M
        // Manual XPA125B 690mV
        bandvoltage = 690;
        // 5
        corrFact = 19;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 3:  // 40M
        // Manual XPA125B 920mV
        bandvoltage = 920;
        // 8
        corrFact = 26;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 4:  // 30M
        // Manual XPA125B 1150mV
        bandvoltage = 1150;
        // 8
        corrFact = 30;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 5:  // 20M
        // Manual XPA125B 1380mV
        bandvoltage = 1380;
        // 9
        corrFact = 35;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 6:  // 17M
        // Manual XPA125B 1610mV
        bandvoltage = 1610;
        // 9
        corrFact = 44;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 7:  // 15M
        // Manual XPA125B 1840mV
        bandvoltage = 1840;
        // 10
        corrFact = 58;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 8:  // 12M
        // Manual XPA125B 2070mV
        bandvoltage = 2070;
        // 12
        corrFact = 55;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 9:  // 10M
        // Manual XPA125B 2300mV
        bandvoltage = 2300;
        // 15
        corrFact = 63;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 10:  // 6M
        // Manual XPA125B 2530mV
        bandvoltage = 2530;
        // 15
        corrFact = 71;
        bandvoltage = bandvoltage + corrFact; // add corrFact for new bandvoltage for calculate
        break;
    case 11:  // NDEF
        bandvoltage = 0;
        break;
    }
    Serial.print("Bandvoltage in mV for XIEGU-PA with LOAD: "); Serial.println(bandvoltage);
    Serial.print("Correct factor in mV                    : "); Serial.println(corrFact);

    sendDAC = bandvoltage * 1024 / 3300; // a value at 1024 is 3V3 output without load 
    // check if the value not greater as 1024 because we use a resolution of 10bit = 2^10 = 1024 = 3V3
    if (sendDAC > 1024) {
      sendDAC = 1024;
    }  

    Serial.print("Send value to DAC: "); Serial.println(sendDAC);
    Serial.println("---------------------------------------------");    
    
    // set analog voltage for bandswitching on PA for XIEGU protocol, e.g. XPA125B or Micro PA50 at PIN 27
    ledcWrite(ledChannel, sendDAC); // send to ESP32-DAC and set bandvoltage
    show_Band_TFT(); //Show frequency in kHz and band in Meters (80m etc) on TFT
    } else {
    // if UART data not start with FA
    Serial.println("invalid UART data from HL2");
    }
  }
 }
}

// Task 2 PTT loop at Core 0
void PTTLoop( void * pvParameters) {
  boolean PTTState;

// start program loop 
for(;;) {

    PTTState = digitalRead(25);

      if ((PTTState == 0) && !(LastState == 0)) {
        digitalWrite(26,HIGH); // PTT via opto-coupler ON
        Serial.println("PTT ON");
        // Draw_TX_TFT();
      }
      if ((PTTState == 1) && !(LastState == 1)) {
        digitalWrite(26,LOW); // PTT via opto-coupler OFF
        Serial.println("PTT OFF");
        // Draw_RX_TFT();       
      }
    LastState = PTTState; // remember last PTT status
    delay(75); // we need a little delay or break in loop, otherwise the task managment will be broken and the ESP32 has a boot loop
  }
}
