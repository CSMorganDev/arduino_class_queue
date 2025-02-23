# Arduino Classroom Queue System

## Overview
This Arduino application manages a classroom queue system using an LED matrix display and MQTT for communication. It allows students to join a queue, cancel their request, fetch queue updates, and notify when the instructor is away from the desk. The system interacts with an MQTT broker to receive and publish updates.

## Features
- **WiFi Connectivity**: Connects to a WiFi network to communicate with the MQTT broker.
- **MQTT Communication**: Subscribes to an MQTT topic to receive student requests and publishes queue updates.
- **LED Matrix Display**: Displays the current ticket number, student name, and student number.
- **Queue Management**: Handles adding, removing, and updating students in the queue.
- **Away Mode**: Notifies students when the instructor is away.

## Hardware Requirements
- ESP32 or compatible WiFi-enabled microcontroller
- 64x32 RGB LED Matrix
- WiFi network

## Software Requirements
- Arduino IDE
- Required libraries:
  - `WiFi.h`
  - `Adafruit_MQTT.h`
  - `Adafruit_MQTT_Client.h`
  - `ArduinoJson.h`
  - `Adafruit_GFX.h`
  - `P3RGB64x32MatrixPanel.h`
  
## Installation & Setup
1. Clone or download this repository.
2. Install the required libraries in the Arduino IDE.
3. Update `config.h` with WiFi and MQTT broker credentials.
4. Upload the code to your ESP32.

## MQTT Topics
- **Subscription:** `AIO_USERNAME/feeds/AIO_FEED_SUBSCRIBE`
- **Publishing:** `AIO_USERNAME/feeds/AIO_FEED_UPDATE`

## Functionality
- **Connecting to WiFi**: The `wifi_connect()` function establishes a WiFi connection.
- **Connecting to MQTT**: The `MQTT_connect()` function ensures a stable connection to the broker.
- **Handling MQTT Messages**: The `loop()` function listens for incoming messages and processes them accordingly.
- **Displaying Information**: The LED matrix displays the current queue status, including student details and ticket numbers.

## Message Handling
- **Add Student**: Adds a new student to the queue.
- **Cancel Ticket**: Removes a student from the queue.
- **Next Ticket**: Moves to the next student in the queue.
- **Fetch Queue**: Sends an update request.
- **Away Mode**: Toggles the instructor's away status.


