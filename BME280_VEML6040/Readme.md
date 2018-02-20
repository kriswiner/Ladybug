**Practical Environmental Data Logger**

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

I last restarted the experiment after including the enable/disable trick for the VEML6040 sensor on December 27, 2016 at 11:33 AM. As of January 22, 2017 at ~1:00 PM it has been running for 26 days and 1.5 hours for an average current usage of < 240 uA. At this rate, using a 350 mAH LiPo battery instead, I should be able to remotely collect data and store it on a low-power SPI NOR flash memory for > 1450 hours or > 60 days. Of course, I could just add a bigger battery but I calculated that the 24 bytes I need to account for all of the information provided by the sensors every five seconds would fill up a 16 MByte SPI flash memory in about 900 hours. If the average current drops to 167 uA or less, then the 150 mAH LiPo will be right-sized for the 16 MByte SPI flash and practical environmental data logging, which allows long-term data collection in a very small package at a very economical cost, will have been achieved!

Toting up the sources of current I would expect the average current to be ~100 uA or thereabouts so the experiment could go on for quite a while yet. 

In the end, I ran this add-on mounted on a Ladybug development board powered by a 150 mAH LiPo battery for more than 43 days updating the Sharp memory display at 5 second intervals with pressure, altitude, humidity, temperature, red, green, blue ambient light intensity and time and date. The average power required to do this was < 150 uA.

**10/04/2017 Update** I added a Macronix MX25128FZNI 16 MB SPI flash to the experiment and made some other changes. I cut the VEML6040 integration time down to 40 ms to save ~5 uA. I am running the BME280 in forced mode once every five seconds which saves tens of uA, and I am putting the SPI flash in low power mode just after, and taking it out of low power mode just before, every write. I am also ending SPI (SPI.end();) just before I enter STM32.stop mode and initializing SPI (SPI.begin();) when emerging from stop mode.

These changes drop the measured power while in stop mode to 19.5 uA (4.99 out of every 5 seconds) and the average power to <44 uA as far as I can estimate it. So I expect the same experiment with the addition of the SPI flash for data logging to last three times as long, 20 weeks or ~5 months, using the same 150 mAH LiPo as the first experiment. But the SPI flash will only last 38 days at a five second interval so I went ahead and changed the interval to 20 seconds. This means the average current will be somewhat less than 30 uA (19.5 + 25/4 ~ 26 uA) but the flash will last for 150 days. I expect the flash to give out before the battery, we'll see. I will update in mid-March....

**02/20/2018 Update** The board power (LiPo battery) gave out yesterday after 138 days. This was a bit shorter duration than I expected. Average power usage was ~45 uA which is about 50% more than what I was expecting (assuming the LiPo really is 150 mAH). In the meantime I bought an [ST Power Shield](http://www.st.com/en/evaluation-tools/x-nucleo-lpm01a.html) and measured the stop current as 11.1 uA and the average current at 5 second update period of 47.0 uA and at 20 second update period of 20.0 uA. So in the test I ended up using more than twice the power I measure with the power shield. This strange to say the least.

One possibility is the battery was not fully charged, although I am sure I charged it before the test. Another is that it is not actually 150 mAH capacity. Still a third is self discharge (i.e., leakage through internal resistance)  which over 138 days at ~5% per month might account for an extra ~9 uA of current usage but not the entire extra ~25 uA expended. The self-discharge would have to be closer to 12.5% to account for this amount, or maybe a combination of self-discharge and incomplete charging could account for the reult.

The good news is the Ladybug plus sensors plus SPI flash plus display is ultra-low  power. The bad news is that the battery might not be!

I did manage to record 54,000 256-byte pages of data on the SPI flash memory. This is a lot of data and my attempts to plot all of it in open office caused the program to fail. So eventually I output only every 15th data entry (every five minutes) and was able to plot all of the data with minimum fuss. Here are some examples:

![pressure]()
