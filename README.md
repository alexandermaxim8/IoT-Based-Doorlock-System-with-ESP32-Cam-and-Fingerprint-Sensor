# IoT-Based-Doorlock-System-with-ESP32-Cam-and-Fingerprint-Sensor
![image](https://github.com/alexandermaxim8/IoT-Based-Doorlock-System-with-ESP32-Cam-and-Fingerprint-Sensor/assets/143409662/652d937d-a10d-4ac7-a7fd-e39505b75ec1)

This project is a further continuity based on Smart Attendance System project. Basically, this system consists of two parallel working parts to activate the door lock, the Smart Attendance Systemm added with guest camera using ESP32-CAM connected to a Web Server.

## Components and Architecture
![image](https://github.com/alexandermaxim8/IoT-Based-Doorlock-System-with-ESP32-Cam-and-Fingerprint-Sensor/assets/143409662/711556fa-7821-4e17-b097-7d3b077a90fc)
- ESP32 Devkit V1
- ESP32-CAM AI Thinker
- ZFM20 Fingerprint Sensor
- LCD 16x2
- SD Card Adapter Module
- Active Buzzer
- Push Button
- 10K Resistor
- 100Ohm Resistor
- IC 7432
- IR Sensor
- 5mm LED
- 5V Relay Module
- 12V Solenoid Actuator

## Operation
To active the door lock solenoid, it has two conditions input via IC7432 OR gate. Firstly, it is a fingerprint detection system which would detect a pre-registered fingerprint. As it detects, the relay driven actuator will be turned-on for 8 seconds. 
For the detail operation of Smart Attendance System, please kindly refer to [this link](https://github.com/alexandermaxim8/-Smart-Attendance-System/tree/main)

And then, the subsequent part is the ESP32-CAM system. In essence, in this system was built a web-server which contains essential features like an image display, toggle button, plus refresh and optional capture buttons. IR Sensor acts a bell so when the guest their hand close, ESP32-CAM would take a picture and afterwards send local-hosted website link via Whatsapp chat. In parallel, the website would served an updated image capture of the guest. The house owner may decide for the lock to be opened or not using the toggle widget. 

## Result
[https://youtu.be/4blCHpZiUhY](https://youtu.be/4blCHpZiUhY)
![image](https://github.com/alexandermaxim8/IoT-Based-Doorlock-System-with-ESP32-Cam-and-Fingerprint-Sensor/assets/143409662/2526a185-81f2-4169-823d-4184a0290aa0)


## Unsolved Issues
Relay switching action from 1 to 0 is effecting the Smart Attendance System part to be forcingly rebooted. Likely caused by not enough of available power for the whole system, absorbed by the solenoid actuator.




