 
STM32 POC

    STM32 POC Definition :- 
STM32F103C8 MCU collect Temperature data from DS18B20 Temperature sensor and get current Date & Timestamp from DS1307 RTC module Realtime Temperature data with  Date & Timestamp data store in sdcard and send Temperature data to cloud using sim800a GSM module.

 Schematic Diagram

https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot1.png


DS18B20 Temperature sensor connect with stm32 MCU using 1 wire bus protocol
DS1307 RTC module connect with stm32 MCU using I2C protocol
SD card module connect with stm32 MCU using SPI protocol 
SIM800 GSM module connect with stm32 MCU using UART protocol 
SD card module using FatFs library for perform read,write operation with stm32 MCU
SIM800 GSM module send real-time Temperature data to Thingspeak cloud platform 
Kalman Filter formula applied for reduce inaccuracies from Temperature value and to predict accurate Temperature value

Working :-

STM32 MCU first initialize and configure clocks and GPIO port for working with external sensors and devices.STM32 read temperature data every second interval and get current date and time from DS1307 RTC module and store temperature with date & time stamp data into sd card csv file.
SD card adapter module first time initialize with STM32 MCU then STM32 check for program defined csv file name in sd card if it's not there then stm32 will create it and write temperature and date & time stamp data into csv file.when stm32 get new data it append into csv file.
STM32 initialize SIM800 GSM module GPRS and HTTP services for sending temperature sensor data over cloud by sending AT commands over UART communication protocol to SIM800 GSM module.

Code :-

 This following code first configure clock and enable GPIO port then initialize UART communication and send AT commands for configure GPRS services and initialize RTC module.

https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot2.png

This following code initialize sd card with STM32 and create csv file if not present in sd card.

https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot3.png

This following code get current Date & Time from RTC module and get temperature data from sensor and write  temperature and date and time stamp data into csv file.

https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot4.png


This following code stm32 send AT commands to GSM module for intialize http service and send temperature data to cloud.


https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot5.png

 This following code apply kalman filter formula on temperature data and give filter value
 

https://github.com/simformsolutions/STM32-Temperaturelogger-IOT/blob/master/screenshots/screenshot6.png
