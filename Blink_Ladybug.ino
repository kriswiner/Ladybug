/* LED Blink, for Ladybug

   This example code is in the public domain.
*/
#include <Arduino.h>
#include <Wire.h>

#define myLed1 13 // orange led

float VDDA, Temperature;

void setup() 
{
  Serial.begin(38400);
 // while (!SerialUSB) { }
  delay(2000);
  Serial.println("Serial enabled!");
 
  pinMode(myLed1, OUTPUT);
  digitalWrite(myLed1, LOW);  // start with leds off, since active HIGH
}

void loop() 
{
  digitalWrite(myLed1, !digitalRead(myLed1)); // toggle red led on
  delay(100);                                    // wait 1 second
  digitalWrite(myLed1, !digitalRead(myLed1)); // toggle red led off
  delay(1000);

  VDDA = STM32.getVREF();
  Temperature = STM32.getTemperature();

  Serial.print("VDDA = "); Serial.println(VDDA, 2); 
  Serial.print("STM32L4 MCU Temperature = "); Serial.println(Temperature, 2);
}
