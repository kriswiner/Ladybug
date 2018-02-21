/* 06/26/2017 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 * 
 
 The BME280 is a simple but high resolution pressure/humidity/temperature sensor, which can be used in its high resolution
 mode but with power consumption of 20 microAmp, or in a lower resolution mode with power consumption of
 only 1 microAmp. The choice will depend on the application.

 VEML6040 color sensor senses red, green, blue, and white light and incorporates photodiodes, amplifiers, 
 and analog / digital circuits into a single chip using CMOS process. With the   color   sensor   applied,   
 the   brightness,   and   color temperature of backlight can be adjusted base on ambient light  source  
 that  makes  panel  looks  more  comfortable  for  end   user’s   eyes.   VEML6040’s   adoption   of   FiltronTM
 technology  achieves  the  closest  ambient  light  spectral  sensitivity to real human eye responses.

 VEML6040  provides  excellent  temperature  compensation  capability  for  keeping  the  output  stable  
 under  changing  temperature.   VEML6040’s   function   are   easily   operated   via the simple command format 
 of I2C (SMBus compatible) interface  protocol.  VEML6040’s  operating  voltage  ranges  from   2.5   V   to   
 3.6   V.   VEML6040   is   packaged   in   a   lead  (Pb)-free  4  pin  OPLGA  package  which  offers  the  best market-proven reliability.
 
  Library may be used freely and without limit with attribution.
 
  */
#include "Wire.h"   
#include <RTC.h>
#include <SPI.h>
#include "BME280.h"
#include "VEML6040.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

// define pins for Sharp LCD display, any pins can be used
uint8_t DSCK  = 5;
uint8_t DMOSI = 8;
uint8_t DSS   = 9;

Adafruit_SharpMem display(DSCK, DMOSI, DSS);

#define BLACK 0
#define WHITE 1

#define SerialDebug true  // set to true to get Serial output for debugging
#define myLed 13

// Specify BME280 configuration
uint8_t Posr = P_OSR_16, Hosr = H_OSR_16, Tosr = T_OSR_02, Mode = normal, IIRFilter = BW0_042ODR, SBy = t_1000ms;     // set pressure amd temperature output data rate

int32_t rawPress, rawTemp, compHumidity, compTemp, compPress;   // pressure, humidity, and temperature raw count output for BME280
int16_t rawHumidity;  // variables to hold raw BME280 humidity value

float temperature_C, temperature_F, pressure, humidity, altitude; // Scaled output of the BME280

BME280 BME280; // instantiate BME280 class

// Specify VEML6040 Integration time
/*Choices are:
 IT_40  40 ms, IT_80 80 ms, IT_160  160 ms, IT_320  320 ms, IT_640 640 ms, IT_1280 1280 ms*/uint8_t IT = IT_160;  // integration time variable
uint8_t ITime = 160;  // integration time in milliseconds
int16_t RGBWData[4] = {0, 0, 0, 0};
float GSensitivity = 0.25168/((float) (IT + 1)); // ambient light sensitivity increases with integration time
float redLight, greenLight, blueLight, ambientLight;

VEML6040 VEML6040;

uint32_t delt_t = 0, count = 0, sumCount = 0, slpcnt = 0;  // used to control display output rate

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 16;
const uint8_t hours = 9;

/* Change these values to set the current initial date */
const byte day = 27;
const byte month = 12;
const byte year = 16;

uint8_t Minutes, Hours;

void setup()
{
  Serial.begin(115200);
  delay(4000);

  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH);
  
  Wire.begin(); // set master mode 
  Wire.setClock(400000); // I2C frequency at 400 kHz  
  delay(1000);

  BME280.I2Cscan(); // should detect BME280 at 0x76

  // Set up for data display
  display.begin(); // Initialize the display
  display.setTextSize(1); // Set text size to normal, 2 is twice normal etc.
  display.setTextColor(BLACK); // Set pixel color; 1 on the monochrome screen
  display.clearDisplay();   // clears the screen and buffer

// Start device display with ID of sensor
  display.setCursor(0, 10); display.print("Ladybug");
  display.setCursor(0, 20); display.print("BME280");
  display.setCursor(0,40); display.print("VEML6040");
  display.setCursor(0,60); display.print("SharpDisplay");
  display.refresh();
  delay(1000);

  // Set the time
  RTC.setHours(hours);
  RTC.setMinutes(minutes);
  RTC.setSeconds(seconds);

  // Set the date
  RTC.setDay(day);
  RTC.setMonth(month);
  RTC.setYear(year);
  
  // Read the WHO_AM_I register of the BME280 this is a good test of communication
  byte f = BME280.getChipID();  // Read WHO_AM_I register for BME280
  Serial.print("BME280 "); Serial.print("I AM "); Serial.print(f, HEX); Serial.print(" I should be "); Serial.println(0x60, HEX);
  Serial.println(" ");
  
  display.clearDisplay();
  display.setCursor(20,0); 
  display.print("BME280");
  display.setCursor(0,10); 
  display.print("I AM");
  display.setCursor(0,20); 
  display.print(f, HEX);  
  display.setCursor(0,30); 
  display.print("I Should Be");
  display.setCursor(0,40); 
  display.print(0x60, HEX);  
  display.refresh();
  delay(1000); 
  
  if(f == 0x60) {
   
    BME280.resetBME280(); // reset BME280 before initilization
    delay(100);

    BME280.BME280Init(Posr, Hosr, Tosr, Mode, IIRFilter, SBy); // Initialize BME280 altimeter
  }
  else Serial.println(" BME280 not functioning!");

    VEML6040.enableVEML6040(IT); // initalize sensor
    delay(150);  
}

void loop()
{
    // BME280 Data
    rawTemp =  BME280.readBME280Temperature();
    compTemp = BME280.BME280_compensate_T(rawTemp);
    temperature_C = (float) compTemp/100.0f;
    temperature_F = 9.0f*temperature_C/5.0f + 32.0f;
     
    rawPress =  BME280.readBME280Pressure();
    compPress = BME280.BME280_compensate_P(rawPress);
    pressure = (float) compPress/25600.f; // Pressure in mbar
    altitude = 145366.45f*(1.0f - powf((pressure/1013.25f), 0.190284f));   
   
    rawHumidity =  BME280.readBME280Humidity();
    compHumidity = BME280.BME280_compensate_H(rawHumidity);
    humidity = (float)compHumidity/1024.0f; // Humidity in %RH

      Serial.println("BME280:");
      Serial.print("Altimeter temperature = "); 
      Serial.print( temperature_C, 2); 
      Serial.println(" C"); // temperature in degrees Celsius
      Serial.print("Altimeter temperature = "); 
      Serial.print(9.0f*temperature_C/5.0f + 32.0f, 2); 
      Serial.println(" F"); // temperature in degrees Fahrenheit
      Serial.print("Altimeter pressure = "); 
      Serial.print(pressure, 2);  
      Serial.println(" mbar");// pressure in millibar
      altitude = 145366.45f*(1.0f - powf((pressure/1013.25f), 0.190284f));
      Serial.print("Altitude = "); 
      Serial.print(altitude, 2); 
      Serial.println(" feet");
      Serial.print("Altimeter humidity = "); 
      Serial.print(humidity, 1);  
      Serial.println(" %RH");// pressure in millibar
      Serial.println(" ");

    // VEML6040 Data
    VEML6040.enableVEML6040(IT); // enable VEML6040 sensor
    delay(ITime);  // wait for integration of light sensor data
    VEML6040.getRGBWdata(RGBWData); // read light sensor data
    VEML6040.disableVEML6040(IT); // disable VEML6040 sensor
 
      Serial.print("Red raw counts = ");   Serial.println(RGBWData[0]);
      Serial.print("Green raw counts = "); Serial.println(RGBWData[1]);
      Serial.print("Blue raw counts = ");  Serial.println(RGBWData[2]);
      Serial.print("White raw counts = "); Serial.println(RGBWData[3]);
      Serial.print("Inferred IR raw counts = "); Serial.println(RGBWData[3] - RGBWData[0] - RGBWData[1] - RGBWData[2]);
      Serial.println("  ");
 
      Serial.print("Red   light power density = "); Serial.print((float)RGBWData[0]/96.0f, 2); Serial.println(" microWatt/cm^2");
      Serial.print("Green light power density = "); Serial.print((float)RGBWData[1]/74.0f, 2); Serial.println(" microWatt/cm^2");
      Serial.print("Blue  light power density = "); Serial.print((float)RGBWData[2]/56.0f, 2); Serial.println(" microWatt/cm^2");
      Serial.println("  ");

      Serial.print("Ambient light intensity = "); Serial.print((float)RGBWData[1]*GSensitivity, 2); Serial.println(" lux");
      Serial.println("  ");

  // Empirical estimation of the correlated color temperature CCT:
  // see https://www.vishay.com/docs/84331/designingveml6040.pdf
      float temp = ( (float) (RGBWData[0] - RGBWData[2])/(float) RGBWData[1] );
      float CCT = 4278.6f*pow(temp, -1.2455f) + 0.5f;

      Serial.print("Correlated Color Temperature = "); Serial.print(CCT); Serial.println(" Kelvin");
      Serial.println("  ");

      display.clearDisplay();
      display.setCursor(0,0); display.print("P "); display.print(pressure); display.print(" mbar");
      display.setCursor(0,10); display.print("Alt "); display.print(altitude, 1); display.println(" ft");
      display.setCursor(0,20); display.print("T "); display.print(temperature_C, 1); display.print(" C "); display.print(9.0f*(temperature_C)/5.0f + 32.0f, 1); display.println(" F");
      display.setCursor(0,30); display.print("rH "); display.print(humidity); display.print(" %rH");
      display.setCursor(0,40); display.print("R "); display.print((float)RGBWData[0]/96.0f); display.print(" mW/cm2");
      display.setCursor(0,50); display.print("G "); display.print((float)RGBWData[1]/74.0f); display.print(" mW/cm2 ");  
      display.setCursor(0,60); display.print("B "); display.print((float)RGBWData[2]/56.0f); display.print(" mW/cm2");
      display.setCursor(0,70); display.print(RTC.getMonth()); display.print("/"); display.print(RTC.getDay()); display.print("/"); display.print(RTC.getYear());     
      Minutes = RTC.getMinutes();
      Hours   = RTC.getHours();     
      display.setCursor(0,80); 
      if(Hours < 10) {display.print("0"); display.print(Hours);} else display.print(Hours);
      display.print(":"); 
      if(Minutes < 10) {display.print("0"); display.print(Minutes);} else display.print(Minutes); 
      display.print(":"); display.print(RTC.getSeconds());  
      display.refresh();
      
      digitalWrite(myLed, HIGH); // flash the led
      delay(1);
      digitalWrite(myLed, LOW);
      count = millis();
      STM32.stop(5000);    // time out in stop mode to save power
}
 
