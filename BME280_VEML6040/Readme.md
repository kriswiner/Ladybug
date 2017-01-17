Practical Environmental Data Logger

It has been a long standing goal of mine to create an environmental sensor/data logger that could run on a small battery for long enough to collect data from a remote location unattended for a few weeks for later analysis.

I tried first using the Teensy 3.1 with [this](https://www.tindie.com/products/onehorse/m41t62-real-time-clock-add-on-for-teensy-3x/?pt=full_prod_search) add-on, but the best I could do was about 24 hours or so on a 750 mAH LiPo due to the ~30 mA current draw of the MK20 MCU. Now to be fair, I was never quite able to get the SNOOZE library to work the way I wanted and it is possible one could do better today. But the low power modes available for the Teensy are limited.

I then tried the ESP8266/85, Espressif's wonderchip with the embedded wifi. Now, wifi has a reputation as a power hog and it is true in default mode, but I figured out how to invoke the light sleep mode on the ESP8266/85 and, with [this](https://www.tindie.com/products/onehorse/esp8285-development-board/?pt=full_prod_search) device, got the average power usage down to ~10 mA or so. With the device broadcasting pressure, humidity, temperature, and light level every 30 seconds I was able to get out to the same 24 hour duration with a 300 mAH battery. Progress but still not what I wanted.

Now the STM32L4 is specifically designed for low power applications, and I am in the middle of my first really successful experiment. I designed an add-on board with a BME280 pressure, temperature, humidity sensor and a VEML6040 red, green, blue, white ambient light sensor to mount on the smallest STM32L4 development board, Ladybug. I am using a few tricks to drop the power usage as low as I can while preserving the state information (not requiring reinitialization of all sensors and registers).

One is I am taking advantage of the STM32.stop(timeout) command at the end of the main loop to suspend the MCU in between data read, processing, and display for five seconds. Of course, the longer the timeout, the lower will be the average current. But for slowly varying data like pressure temperature, humidity and light level, a five second reporting interval seems quite reasonable to me.

The other is I am taking advantage of the enable/disable functionality of the VEML6040, since it is the big power user of the two sensors at an average current of 200 uA (this is large in the world of low power!). At the start of the main loop I enable the VEML6040 sensor, wait for a conversion time of 160 msec, read the data, then disable the sensor. This drops the average power usage to ~200 x (200/5000) ~ 8 uA, still higher than the ~4 uA of the BME280 sensor running full blast but a lot lower than 200 uA.

The other nice thing about the STM32L4 is that the embedded RTC allows a time and date stamp for the data, and the RTC can be calibrated for better than 1 ppm accuracy. In my experiments with the RTC uncalibrated it is gaining about one second per day.

This is what the environmental sensor experiment looks like:
![data logger](https://cdn.hackaday.io/images/996711484502329867.jpg)

You can see the add-on board with the sensors mounted on female headers on top of the Ladybug, itself stuck in a small breadboard with a Sharp memory display to show the data, all powered by a 150 mAH LiPo battery.

I last restarted the experiment after including the enable/disable trick for the VEML6040 sensor on December 27, 2016 at 11:33 AM. As of today January 15, 2017 at ~10 AM it has been running for 18 days and 22.5 hours for an average current usage of ~330 uA. At this rate, using a 350 mAH LiPo battery instead, I should be able to remotely collect data and store it on a low-power SPI NOR flash memory for > 1000 hours or 44 days. Of course, I could just add a bigger battery but I calculated that the 24 bytes I need to account for all of the information provided by the sensors every five seconds would fill up a 16 MByte SPI flash memory in about 900 hours. So this device looks like my first practical environmental data logger which allows long-term data collection in a very small package at a very economical cost.

Toting up the sources of current I would expect the average current to be ~100 uA or thereabouts so the experiment could go on for quite a while yet. I will update every few days until the battery conks out.
