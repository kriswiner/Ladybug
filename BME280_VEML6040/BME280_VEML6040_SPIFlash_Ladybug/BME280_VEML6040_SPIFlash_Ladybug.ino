/* BME280 and VEML6040 Basic Example Code using Ladybug
 by: Kris Winer
 date: December 3, 2016
 license: Beerware - Use this code however you'd like. If you 
 find it useful you can buy me a beer some time.
 
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
 
 SDA and SCL should have 4K7 pull-up resistors (to 3.3V).
 
 Hardware setup:
 SDA ----------------------- 21
 SCL ----------------------- 22
 
  */
#include "Wire.h"   
#include <RTC.h>
#include <SPI.h>
#include "SPIFlash.h"
#include "BME280.h"
#include "VEML6040.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>

const char        *build_date = __DATE__;   // 11 characters MMM DD YYYY
const char        *build_time = __TIME__;   // 8 characters HH:MM:SS

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
uint8_t Posr = P_OSR_01, Hosr = H_OSR_01, Tosr = T_OSR_01, Mode = sleepMode, IIRFilter = full, SBy = t_1000ms;     // set pressure amd temperature output data rate

float Temperature, Pressure, Humidity; // stores BME280 pressures sensor pressure and temperature
uint32_t rawPress, rawTemp, compHumidity, compTemp, compPress;   // pressure and temperature raw count output for BME280
uint16_t rawHumidity;  // variables to hold raw BME280 humidity value
float   temperature_C, temperature_F, pressure, humidity, altitude; // Scaled output of the BME280

BME280 BME280; // instantiate BME280 class

// Specify VEML6040 Integration time
/*Choices are:
 IT_40  40 ms, IT_80 80 ms, IT_160  160 ms, IT_320  320 ms, IT_640 640 ms, IT_1280 1280 ms*/
uint8_t IT = IT_40;  // integration time variable
uint8_t ITime = 40;  // integration time in milliseconds
uint16_t RGBWData[4] = {0, 0, 0, 0};
float GSensitivity = 0.25168/((float) (1 << IT)); // ambient light sensitivity increases with integration time
float redLight, greenLight, blueLight, ambientLight;

VEML6040 VEML6040;

/* Change these values to set the current initial time */
uint8_t seconds = 0;
uint8_t minutes = 58;
uint8_t hours = 18;

/* Change these values to set the current initial date */
uint8_t day = 4;
uint8_t month = 10;
uint8_t year = 17;

uint8_t Seconds, Minutes, Hours, Day, Month, Year;

bool alarmFlag = false;

// 128 MBit (16 MByte) SPI Flash 65536, 256-byte pages
#define csPin 10 // SPI Flash chip select pin

uint16_t page_number = 0;     // set the page number for flash page write
uint8_t  sector_number = 0;   // set the sector number for sector write
uint8_t  flashPage[256];      // array to hold the data for flash page write

SPIFlash SPIFlash(csPin);

void setup()
{
  Serial.begin(115200);
  delay(4000);

  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH);
  
  Wire.begin(); // set master mode 
  Wire.setClock(400000); // I2C frequency at 400 kHz  
  delay(1000);

  BME280.I2Cscan(); // should detect BME280 at 0x77, BMA280 at 0x18, and CCS811 at 0x5A

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
//  SetDefaultRTC();

  RTC.setDay(day);
  RTC.setMonth(month);
  RTC.setYear(year);
  RTC.setHours(hours);
  RTC.setMinutes(minutes);
  RTC.setSeconds(seconds);
  RTC.setDay(day);
  RTC.setMonth(month);
  RTC.setYear(year);
  RTC.setHours(hours);
  RTC.setMinutes(minutes);
  RTC.setSeconds(seconds);
  
  // Read the WHO_AM_I register of the BME280 this is a good test of communication
  byte f = BME280.getChipID();  // Read WHO_AM_I register for BME280
  Serial.print("BME280 "); Serial.print("I AM "); Serial.print(f, HEX); Serial.print(" I should be "); Serial.println(0x60, HEX);
  Serial.println(" ");
  display.clearDisplay();
  display.setCursor(20,0); display.print("BME280");
  display.setCursor(0,10); display.print("I AM");
  display.setCursor(0,20); display.print(f, HEX);  
  display.setCursor(0,30); display.print("I Should Be");
  display.setCursor(0,40); display.print(0x60, HEX);  
  display.refresh();
  delay(1000); 
  
  if(f == 0x60) {
   
   BME280.resetBME280(); // reset BME280 before initilization
   delay(100);

   BME280.BME280Init(Posr, Hosr, Tosr, Mode, IIRFilter, SBy); // Initialize BME280 altimeter
 }
  else Serial.println(" BME280 not functioning!");

  // check SPI Flash ID
  SPIFlash.SPIFlashinit();
  SPIFlash.getChipID();
//  SPIFlash.flash_chip_erase(true); // full erase

  VEML6040.enableVEML6040(IT); // initalize sensor
  delay(150);  

  // set alarm to update the RTC every second
//  RTC.enableAlarm(RTC.MATCH_ANY); // alarm once a second
  
//  RTC.attachInterrupt(alarmMatch);
}

void loop()
{
//    if(alarmFlag) { // update RTC output (serial display) whenever the RTC alarm condition is achieved
//       alarmFlag = false;

    // BME280 Data
    BME280.forced();  // get one data sample, then go back to sleep

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
    delay(ITime);  // wait for integration of light sensor data + 20 ms
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
    Day =     RTC.getDay();
    Month =   RTC.getMonth();
    Year =    RTC.getYear();
    Minutes = RTC.getMinutes();
    Hours   = RTC.getHours();
    Seconds = RTC.getSeconds();     
      display.setCursor(0,70); display.print(Month); display.print("/"); display.print(Day); display.print("/"); display.print(Year);     
      display.setCursor(0,80); 
      if(Hours < 10) {display.print("0"); display.print(Hours);} else display.print(Hours);
      display.print(":"); 
      if(Minutes < 10) {display.print("0"); display.print(Minutes);} else display.print(Minutes); 
      display.print(":"); 
      if(Seconds < 10) {display.print("0"); display.print(Seconds);} else display.print(Seconds); 
      display.refresh();

      // store some data to the SPI flash
      // 25 bytes of data means 10 sectors of 25 bytes per 256 byte flash page
    if(sector_number < 10 && page_number < 0xFFFF) {
      flashPage[sector_number*25 + 0]  = (RGBWData[0] & 0xFF00) >> 8; // MSB RGBW data 0
      flashPage[sector_number*25 + 1]  = RGBWData[0] & 0x00FF;        // LSB RGBW data 0
      flashPage[sector_number*25 + 2]  = (RGBWData[1] & 0xFF00) >> 8; // MSB RGBW data 1
      flashPage[sector_number*25 + 3]  = RGBWData[1] & 0x00FF;        // LSB RGBW data 1
      flashPage[sector_number*25 + 4]  = (RGBWData[2] & 0xFF00) >> 8; // MSB RGBW data 2
      flashPage[sector_number*25 + 5]  = RGBWData[2] & 0x00FF;        // LSB RGBW data 2
      flashPage[sector_number*25 + 6]  = (RGBWData[3] & 0xFF00) >> 8; // MSB RGBW data 3
      flashPage[sector_number*25 + 7]  = RGBWData[3] & 0x00FF;        // LSB RGBW data 3 
      flashPage[sector_number*25 + 8] =  (compTemp & 0xFF000000) >> 24;
      flashPage[sector_number*25 + 9] =  (compTemp & 0x00FF0000) >> 16;
      flashPage[sector_number*25 + 10] = (compTemp & 0x0000FF00) >> 8;
      flashPage[sector_number*25 + 11] = (compTemp & 0x000000FF);
      flashPage[sector_number*25 + 12] = (compHumidity & 0xFF000000) >> 24;
      flashPage[sector_number*25 + 13] = (compHumidity & 0x00FF0000) >> 16;
      flashPage[sector_number*25 + 14] = (compHumidity & 0x0000FF00) >> 8;
      flashPage[sector_number*25 + 15] = (compHumidity & 0x000000FF);
      flashPage[sector_number*25 + 16] = (compPress & 0xFF000000) >> 24;
      flashPage[sector_number*25 + 17] = (compPress & 0x00FF0000) >> 16;
      flashPage[sector_number*25 + 18] = (compPress & 0x0000FF00) >> 8;
      flashPage[sector_number*25 + 19] = (compPress & 0x000000FF);
      flashPage[sector_number*25 + 20] = Seconds;
      flashPage[sector_number*25 + 21] = Minutes;
      flashPage[sector_number*25 + 22] = Hours;
      flashPage[sector_number*25 + 23] = Day;
      flashPage[sector_number*25 + 24] = Month;
      sector_number++;
    }
    else if(sector_number == 10 && page_number < 0xFFFF)
    {
      SPIFlash.powerUp();
      SPIFlash.flash_page_program(flashPage, page_number);
      SPIFlash.powerDown();
      Serial.print("Wrote flash page: "); Serial.println(page_number);
      sector_number = 0;
      page_number++;
    }  
    else
    {
      Serial.println("Reached last page of SPI flash!"); Serial.println("Data logging stopped!");
    }
    
//      digitalWrite(myLed, HIGH); // flash the led
//      delay(1);
//      digitalWrite(myLed, LOW);
//    }

      SPI.end();
      STM32.stop(5000);    // time out in stop mode to save power
      SPI.begin();
}

//===================================================================================================================
//====== Set of useful functions
//===================================================================================================================
  
 void alarmMatch()
{
  alarmFlag = true;
}


void SetDefaultRTC()  // Sets the RTC to the FW build date-time...
{
  char Build_mo[3];

  // Convert month string to integer

  Build_mo[0] = build_date[0];
  Build_mo[1] = build_date[1];
  Build_mo[2] = build_date[2];

  String build_mo = Build_mo;

  if(build_mo == "Jan")
  {
    month = 1;
  } else if(build_mo == "Feb")
  {
    month = 2;
  } else if(build_mo == "Mar")
  {
    month = 3;
  } else if(build_mo == "Apr")
  {
    month = 4;
  } else if(build_mo == "May")
  {
    month = 5;
  } else if(build_mo == "Jun")
  {
    month = 6;
  } else if(build_mo == "Jul")
  {
    month = 7;
  } else if(build_mo == "Aug")
  {
    month = 8;
  } else if(build_mo == "Sep")
  {
    month = 9;
  } else if(build_mo == "Oct")
  {
    month = 10;
  } else if(build_mo == "Nov")
  {
    month = 11;
  } else if(build_mo == "Dec")
  {
    month = 12;
  } else
  {
    month = 1;     // Default to January if something goes wrong...
  }

  // Convert ASCII strings to integers
  day     = (build_date[4] - 48)*10 + build_date[5] - 48;  // ASCII "0" = 48
  year    = (build_date[9] - 48)*10 + build_date[10] - 48;
  hours   = (build_time[0] - 48)*10 + build_time[1] - 48;
  minutes = (build_time[3] - 48)*10 + build_time[4] - 48;
  seconds = (build_time[6] - 48)*10 + build_time[7] - 48;

  // Set the date/time

  RTC.setDay(day);
  RTC.setMonth(month);
  RTC.setYear(year);
  RTC.setHours(hours);
  RTC.setMinutes(minutes);
  RTC.setSeconds(seconds);
}


