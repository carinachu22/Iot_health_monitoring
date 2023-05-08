# Iot Health Monitoring 
## Overview of project
Health monitoring device that tracks the heart rate, SpO2, skin temperature, environment temperature and humidity. 
Sensor readings are uploaded in realtime to Firebase Realtime Database and displayed in a web application. 
Web application has cards for the realtime values of the readings, and have line charts plotted to better visualise trends 

## Hardware 
Project is built with an ESP32 board. Sensors are connected via I2C. Sensors used in this project is MPU6050, Sparkfun heart rate and pulse oximeter, AM2320, MAX30205.

## Before running the project 
1. Download the following Arduino libraries
  - Firebase Arduino Client Library for ESP8266 and ESP32
  - Firebase ESP32 Client
  - Any other libraries for your sensors
  
2. Edit the arduino code with your wifi details - name and password 
3. Create your own firebase realtime database
3. Edit the JavaScript file with your own firebase config key. Config key can be obtained from firebase project console 
4. Run `firebase deploy` in your terminal to enable firebase hosting 

## Running the project 
1. Connect all sensors to the ESP32 board following pin configurations defined in the arduino code
2. Upload Arduino code to the ESP32 board 
3. Open Firebase web app to view readings 

