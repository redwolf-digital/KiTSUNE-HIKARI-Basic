/*
HIKARI PROJECT 
BASIC - Aperture priority mode only
by WOLFNEST-Studio
my IG
  @ redwolf_studio
  @ kitsune_film
--------------------------------------------------
Micro controller
 - atmega328p
Sensor
 - BH1750FVI (lux senser)
Displsy
 - 128x32 SSD1206 OLED display
--------------------------------------------------
*/

#include <EEPROM.h>

//Communication bus
#include <Wire.h>
#include <SPI.h>

//Lux sensor Lib [ by PeterEmbedded ]
#include <BH1750FVI.h>
//SSD1306 OLED Display Lib
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Config
#define SCREEN_WIDTH      128
#define SCREEN_HEIGHT     32
#define OLED_RESET        -1

//Value
#define CALIBRATION_CONSTANT_INC  250       //incident  mode
#define CALIBRATION_CONSTANT_REX  12.5      //reflected mode

float lux;
float EV;
float TIME;
float APERTURE;
uint16_t ISO;
uint16_t SHUTTER;


//Pin config
#define measureButt           2
#define settingButt           3
#define apeButt               4
#define ISOButt               5
#define PowerControl          7

//function
boolean TYPE_measure;  // 0 : incident mode   1 : reflected mode
boolean SettingMode = 0;
uint8_t AepSelect;
uint8_t isoSelect;
uint32_t SLEEP_TIME = 25000;      //25 sce.
uint32_t SHUTDOWN_TIME = 60000;   //1 min.
uint8_t Page = 0;

//light sennsor
BH1750FVI LightSensor(BH1750FVI::k_DevModeContLowRes);
//OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned char KiTSUNE [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x30, 0x40, 0x02, 0xe0, 0x80, 0x03, 0x81, 0x00, 0x02, 
  0x03, 0x00, 0x06, 0x02, 0x00, 0x04, 0x02, 0x00, 0x0c, 0x02, 0x80, 0x0c, 0x03, 0x80, 0x08, 0x03, 
  0x00, 0x08, 0x01, 0x80, 0x18, 0x01, 0x80, 0x18, 0x00, 0xc0, 0x1c, 0x00, 0xc0, 0x0d, 0x04, 0xc0, 
  0x0d, 0x44, 0xc0, 0x07, 0x66, 0xc0, 0x07, 0x27, 0xc0, 0x03, 0xb3, 0xc0, 0x03, 0xff, 0x80, 0x03, 
  0xff, 0x80, 0x03, 0xff, 0x80, 0x07, 0xff, 0x80, 0x07, 0xff, 0x80, 0x0f, 0xff, 0x80, 0x0f, 0xff, 
  0x00, 0x3f, 0xff, 0x00, 0x7f, 0xfe, 0x00, 0x3f, 0xfc, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00
};

void setup() {
  pinMode(measureButt , INPUT_PULLUP);
  pinMode(settingButt , INPUT_PULLUP);
  pinMode(apeButt, INPUT_PULLUP);
  pinMode(ISOButt , INPUT_PULLUP);
  pinMode(PowerControl, OUTPUT);
  
  digitalWrite(PowerControl, HIGH);
    
  Serial.begin(9600);
  LightSensor.begin();
  
  //Report Error
  if(!Wire.requestFrom(0x23, 1)) {
    Serial.println(F("[Err01] No light sensor"));
  }
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("[Err02] No display"));
  }

  TYPE_measure   = EEPROM.get(0, TYPE_measure);
  SLEEP_TIME     = EEPROM.get(1, SLEEP_TIME);
  SHUTDOWN_TIME  = EEPROM.get(5, SHUTDOWN_TIME);
  AepSelect      = EEPROM.get(9, AepSelect);
  isoSelect      = EEPROM.get(10, isoSelect);
  APERTURE       = EEPROM.get(11, APERTURE);
  ISO            = EEPROM.get(15, ISO);

  //Serial debug
  Serial.println(F("------------------------------ DEBUG ------------------------------"));
  Serial.println("");
  Serial.println(F(" [READ EEPROM] "));
  Serial.print(F("[TYPE_measure] "));     Serial.println(TYPE_measure);
  Serial.print(F("[SLEEP_TIME] "));       Serial.println(SLEEP_TIME);
  Serial.print(F("[SHUTDOWN_TIME] "));    Serial.println(SHUTDOWN_TIME);
  Serial.print(F("[AepSelect] "));        Serial.println(AepSelect);
  Serial.print(F("[isoSelect] "));        Serial.println(isoSelect);
  Serial.print(F("[APERTURE] "));     Serial.println(APERTURE);
  Serial.print(F("[APERTURE] "));         Serial.println(ISO);
  
  Serial.println("");
  Serial.println(F("------------------------------ DEBUG ------------------------------"));

  
  display.clearDisplay();
  display.display();
  delay(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,10);
  display.setTextSize(2);
  display.println(F("KiTSUNE"));
  display.drawBitmap(95,0, KiTSUNE, 20, 32, SSD1306_WHITE);
  display.display();
  delay(2000);
  display.fillScreen(SSD1306_BLACK);
  display.display();
  
  
}// End void setup

void loop() {

  static unsigned long TIMER_SLEEP = millis();
  static unsigned long TIMER_SHUTDOWN = millis();
  //Auto sleep
  if(digitalRead(measureButt) == HIGH && digitalRead(settingButt) == HIGH && digitalRead(apeButt) == HIGH  && digitalRead(ISOButt) == HIGH) {
    if((millis() - TIMER_SLEEP) > SLEEP_TIME) {
      display.clearDisplay();
      display.display();
       TIMER_SLEEP = millis();
    }
  }else {
     TIMER_SLEEP = millis();
  }

  //Auto shutdown
  if(digitalRead(measureButt) == HIGH && digitalRead(settingButt) == HIGH && digitalRead(apeButt) == HIGH  && digitalRead(ISOButt) == HIGH) {
    if((millis() - TIMER_SHUTDOWN) > SHUTDOWN_TIME) {
      digitalWrite(PowerControl, LOW);
      TIMER_SHUTDOWN = millis();
    }
  }else {
     TIMER_SHUTDOWN = millis();
  }


  /*
   * ****************************************************************
   * *                         Setting                              *
   * ****************************************************************
   */

   while(digitalRead(settingButt) == LOW && SettingMode == false) {
       SettingMode = true;
       delay(250);
   }

   while(SettingMode == true) {
    SETTING();

    while(digitalRead(settingButt) == LOW && SettingMode == true) {
      SettingMode = false;

      EEPROM.put(0, TYPE_measure);
      EEPROM.put(1, SLEEP_TIME);
      EEPROM.put(5, SHUTDOWN_TIME);
      EEPROM.put(9, AepSelect);
      EEPROM.put(10, isoSelect);
      EEPROM.put(11, APERTURE);
      EEPROM.put(15, ISO);
      show_TIME();
      delay(250);
    }
   }


  /*
   * ****************************************************************
   * *                         Measurement                          *
   * ****************************************************************
   */
  
  if(digitalRead(measureButt) == LOW && SettingMode == false) {
    //Get lux
      lux = LightSensor.GetLightIntensity() * 2.17;
      //Get EV
      if(TYPE_measure == 0) {
        EV = (log10(lux * ISO / CALIBRATION_CONSTANT_INC) / log10(2));
      }
      if(TYPE_measure == 1) {
        EV = (log10(lux * ISO / CALIBRATION_CONSTANT_REX) / log10(2));
      }

      TIME = pow(2, EV) / pow(APERTURE, 2);

      if(TIME >= 1.5) {
        SHUTTER = 2;
      }
      if(TIME >= 3) {
        SHUTTER = 4;
      }
      if(TIME >= 6) {
        SHUTTER = 8;
      }
      if(TIME >= 11.5) {
        SHUTTER = 15;
      }
      if(TIME >= 22.5) {
        SHUTTER = 30;
      }
      if(TIME >= 45) {
        SHUTTER = 60;
      }
      if(TIME >= 92.5) {
        SHUTTER = 125;
      }
      if(TIME >= 187.5) {
        SHUTTER = 250;
      }
      if(TIME >= 375) {
        SHUTTER = 500;
      }
      if(TIME >= 750) {
        SHUTTER = 1000;
      }
      if(TIME > 1500) {
        SHUTTER = 2000;
      }
      if(TIME >= 3000) {
        SHUTTER = 4000;
      }
      if(TIME >= 6000) {
        SHUTTER = 8000;
    }

    show_TIME();
    
    EEPROM.put(9, AepSelect);
    EEPROM.put(10, isoSelect);
    EEPROM.put(11, APERTURE);
    EEPROM.put(15, ISO);
    
    delay(250);
  }

  //APERTURE
   if(digitalRead(apeButt) == LOW && SettingMode == false) {
     AepSelect++;
     if(AepSelect >= 29) {
       AepSelect=0;
     }
     switch(AepSelect) {
       case 0 : APERTURE = 0.95;
                 break;
       case 1 : APERTURE = 1.0;
                 break;
       case 2 : APERTURE = 1.1;
                 break;
       case 3 : APERTURE = 1.2;
                 break;
       case 4 : APERTURE = 1.4;
                 break;
       case 5 : APERTURE = 1.6;
                 break;
       case 6 : APERTURE = 1.8;
                 break;
       case 7 : APERTURE = 2.0;
                 break;
       case 8 : APERTURE = 2.2;
                 break;
       case 9 : APERTURE = 2.5;
                 break;
       case 10 : APERTURE = 2.8;
                 break;
       case 11 : APERTURE = 3.2;
                 break;
       case 12 : APERTURE = 3.5;
                 break;
       case 13 : APERTURE = 4.0;
                 break;
       case 14 : APERTURE = 4.5;
                 break;
       case 15 : APERTURE = 5.0;
                 break;
       case 16 : APERTURE = 5.6;
                 break;
       case 17 : APERTURE = 6.3;
                break;
       case 18 : APERTURE = 7.1;
                 break;
       case 19 : APERTURE = 8.0;
                 break;
       case 20 : APERTURE = 9.0;
                 break;
       case 21 : APERTURE = 10;
                 break;
       case 22 : APERTURE = 11;
                 break;
       case 23 : APERTURE = 13;
                 break;
       case 24 : APERTURE = 14;
                 break;          
       case 25 : APERTURE = 16;
                 break;
       case 26 : APERTURE = 18;
                 break;
       case 27 : APERTURE = 20;
                 break;
       case 28 : APERTURE = 22;
                 break;
     }
     show_APE();
     delay(350);
     display.clearDisplay();
     display.display();
   }

   //ISO
   if(digitalRead(ISOButt) == LOW && SettingMode == false) {
     isoSelect++;
     if(isoSelect >= 28) {
       isoSelect=0;
     }
     switch (isoSelect) {
       case 0 : ISO = 12;
                 break;
       case 1 : ISO = 16;
                 break;
       case 2 : ISO = 20;
                 break;
       case 3 : ISO = 25;
                 break;
       case 4 : ISO = 32;
                 break;
       case 5 : ISO = 40;
                 break;
       case 6 : ISO = 50;
                 break;
       case 7 : ISO = 64;
                 break;
       case 8 : ISO = 80;
                 break;
       case 9 : ISO = 100;
                 break;
       case 10 : ISO = 125;
                 break;
       case 11 : ISO = 160;
                break;
       case 12 : ISO = 200;
                 break;
       case 13 : ISO = 250;
                 break;
       case 14 : ISO = 320;
                 break;
       case 15 : ISO = 400;
                 break;
       case 16 : ISO = 500;
                 break;
       case 17 : ISO = 640;
                 break;
       case 18 : ISO = 800;
                 break;
       case 19 : ISO = 1000;
                 break;
       case 20 : ISO = 1250;
                 break;
       case 21 : ISO = 1600;
                 break;
       case 22 : ISO = 2000;
                 break;
       case 23 : ISO = 2500;
                 break;
       case 24 : ISO = 3200;
                 break;
       case 25 : ISO = 4000;
                 break;
       case 26 : ISO = 5000;
                 break;
       case 27 : ISO = 6400;
                 break;
     }
     show_ISO();
     delay(350);
     display.clearDisplay();
     display.display();
   }
  
}// End void loop

void show_ISO() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.print(F("ISO"));
  display.setCursor(1,15);
  display.setTextSize(2);
  display.print(ISO);
  display.display();
}

void show_APE() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.print(F("APERTURE"));
  display.setCursor(1,15);
  display.setTextSize(2);
  display.print(APERTURE);
  display.display();
}

void show_TIME() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.print(F("TIME"));
  display.setCursor(1,15);
  display.setTextSize(2);

  if(TIME >= 11500) {
    display.print(F("OVER"));
  }
  if(TIME < 1.5) {
    display.print(1 / TIME, 2);
  }
  if(TIME >= 1.5 && TIME < 11500) {
    display.print(F("1/"));
    display.print(SHUTTER);
  }

  display.setTextSize(1);

  //Show aperture
  display.setCursor(75, 1);
  display.print(F("f/"));
  display.setCursor(95, 1);
  display.println(APERTURE, 2);

  //Show ISO
  display.setCursor(75, 10);
  display.print(F("ISO"));
  display.setCursor(95, 10);
  display.println(ISO);

  //Show EV
  display.setCursor(75, 20);
  display.print(F("EV"));
  display.setCursor(95, 20);
  display.println(EV);

  
  display.display();
}

void SETTING() {
  if(digitalRead(apeButt) == LOW && digitalRead(measureButt) == HIGH && SettingMode == true) {
    Page++;
    delay(100);    
  }
  if(digitalRead(ISOButt) == LOW && digitalRead(measureButt) == HIGH && SettingMode == true) {
    Page--;
    delay(100);    
  }

  if(Page == 255 || Page == 4) {
    Page = 0;
  }

  display.clearDisplay();
  display.invertDisplay(0);
  display.fillRect(0, 0, 128, 9, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.println(F("SETTING"));
  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,12);
  display.setTextSize(1);
  switch(Page) {
    case 0 : 
      display.println(F("MODE"));
      if(digitalRead(measureButt) == LOW && TYPE_measure == 0) {
        TYPE_measure = 1;
        delay(50);
      }
      if(digitalRead(measureButt) == LOW && TYPE_measure == 1) {
        TYPE_measure = 0;
        delay(50);
      }

      display.setCursor(35,17);
      if(TYPE_measure == 0) {
        display.println(F("incident"));
      }
      if(TYPE_measure == 1) {
        display.println(F("reflected"));
      }
         
      break;
    
    case 1 :
      display.println(F("SLEEP"));
      display.setCursor(65,15);  //40,15
      display.setTextSize(2);
      display.println(SLEEP_TIME/1000.0, 0);
      display.setCursor(93,19);
      display.setTextSize(1);
      display.println(F("SEC."));

     
      if(digitalRead(measureButt) == LOW) {
        SLEEP_TIME = SLEEP_TIME + 5000;
        delay(250);
        if(SLEEP_TIME > 45000) {
          SLEEP_TIME = 15000;
        }
      }
      
      break;
    
    case 2 :
      display.println(F("SHUTDOWN"));
      display.setCursor(55,15);
      display.setTextSize(2);
      display.println(SHUTDOWN_TIME/60000.0, 1);
      display.setCursor(93,19);
      display.setTextSize(1);
      display.println(F("MIN."));

     
      if(digitalRead(measureButt) == LOW) {
        SHUTDOWN_TIME = SHUTDOWN_TIME + 15000;
        delay(250);
        if(SHUTDOWN_TIME > 300000) {
          SHUTDOWN_TIME = 60000;
        }
      }
      
      break; 

    case 3 :
      display.println(F("RESET"));
      display.println(F("press [Measure]"));

      bool reset = 0;
      if(digitalRead(measureButt) == LOW && reset == 0) {
        reset = 1;
        delay(100);
      }
      while(reset == 1) {
        display.clearDisplay();
        display.invertDisplay(1);
        display.setCursor(1,1);
        display.println(F("Are you sure?"));
        display.setCursor(1,13);
        display.println(F("[MEASURE]  - Enter"));
        display.println(F("[APERTURE] - Esc"));
        display.display();

        if(digitalRead(measureButt) == LOW) {
          TYPE_measure      = 0;
          AepSelect         = 10;
          APERTURE          = 2.8;
          isoSelect         = 9;
          ISO               = 100;
          SLEEP_TIME        = 25000;   //25 sce.
          SHUTDOWN_TIME     = 60000;   //1 min.

          for(int i = 0 ; i < EEPROM.length() ; i++) {
          EEPROM.update(i, 0);
          
          display.clearDisplay();
          display.invertDisplay(0);
          display.setCursor(20,3);
          display.println(F("FORMAT EEPROM"));
          display.setCursor(20,15);
          display.print(i);
          display.print(F(" of "));
          display.print(EEPROM.length());
          display.display();
          }
          reset = 0;

        }
        
        if(digitalRead(apeButt) == LOW) {
          reset = 0;
        }
      }
      break;
  }

  display.display();
}
