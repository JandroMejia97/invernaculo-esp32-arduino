# ESP-32s Project with Arduino and PlatformIO

This project consists of an ESP-32s that uses Arduino libraries to receive data (temperature, air/soil humidity, and light intensity) from sensors and send it to an MQTT server on Ubidots. PlatformIO is used as the integrated development environment (IDE).

## Features

- **Temperature and Humidity Monitoring**: Uses the DHT11 sensor to monitor the temperature and air humidity.
- **Soil Moisture Monitoring**: Monitors soil moisture using a capacitive soil moisture sensor.
- **Light Intensity Monitoring**: Measures light intensity with a photoresistor.
- **Automated Climate Control**: Controls fans based on temperature and air humidity, and controls a water pump based on soil moisture levels.
- **Remote Control**: Fans and water pumps can be remotely controlled via MQTT topics through the Ubidots platform.
- **Parallel Tasks**: Each sensor runs in its own FreeRTOS task to ensure efficient monitoring.
- **MQTT Data Transmission**: Data is sent to Ubidots through MQTT for real-time monitoring and control.

## Before start

Before compiling the project, you must create a file named wifi_credentials.h with the following defined macros:

```c++
#define WIFI_SSID_SECRETS "<Network Name>"
#define WIFI_PASSWORD_SECRETS "<Password>"
#define UBIDOTS_TOKEN_SECRETS "<Ubidots Token>"
#define DEVICE_LABEL_SECRETS "<Device name>"
```

| ⚠️ Make sure to replace the example values with your own data before compiling.

## Components

- **Microcontroller**: ESP32
- **Temperature and Humidity Sensor**: DHT11
- **Soil Moisture Sensor**: Capacitive soil moisture sensor (analog)
- **Light Intensity Sensor**: Photoresistor (analog)
- **Fan and Water Pump**: Connected to ESP32 for climate control

## Pin Configuration

| Component       | ESP32 Pin |
|-----------------|-----------|
| DHT11 Sensor    | GPIO 4    |
| Soil Moisture   | GPIO 34   |
| Light Sensor    | GPIO 35   |
| Fan             | GPIO 12   |
| Water Pump      | GPIO 14   |

## Sensor Sampling Intervals

- **Temperature and Humidity**: 10 minutes
- **Soil Moisture**: 15 minutes
- **Light Intensity**: 15 minutes

## Ubidots Setup

- The system connects to Ubidots using MQTT to publish sensor data and subscribe to actuator control topics.
- Ensure that the following variables are configured in your Ubidots dashboard:
  - `temperature`: for temperature readings.
  - `air_humidity`: for air humidity readings.
  - `light`: for light intensity readings.
  - `soil_humidity`: for soil moisture readings.
  - `fan`: for controlling the fan remotely.
  - `water_pump`: for controlling the water pump remotely.

## Code Overview

### Dependencies

Make sure to include the following libraries in your project:

- [Ubidots Arduino MQTT](https://github.com/ubidots/esp32-mqtt)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [Adafruit DHT](https://github.com/adafruit/DHT-sensor-library)
- [Arduino](https://www.arduino.cc/reference/en/libraries/)

## References

- [Connect an ESP32-DevKitC to Ubidots over MQTT](https://help.ubidots.com/en/articles/748067-connect-an-esp32-devkitc-to-ubidots-over-mqtt)
