# ESP-32s Project with Arduino and PlatformIO

This project consists of an ESP-32s that uses Arduino libraries to receive data (temperature, air/soil humidity, and light intensity) through UART and send it to an MQTT server on Ubidots. PlatformIO is used as the integrated development environment (IDE).

## Before start

Before compiling the project, you must create a file named wifi_credentials.h with the following defined macros:

```c++
#define WIFI_SSID_SECRETS "<Network Name>"
#define WIFI_PASSWORD_SECRETS "<Password>"
#define UBIDOTS_TOKEN_SECRETS "<Ubidots Token>"
#define DEVICE_LABEL_SECRETS "<Device name>"
```

| ⚠️ Make sure to replace the example values with your own data before compiling.

## Customization

Additionally, you can customize the project by overwriting the following constants:

```c++
#define DATA_UART <UART NUMBER>
#define SERIAL_RX <PIN TO RECEIVE DATA>
#define SERIAL_TX <PIN TO TRANSMIT DATA>
#define BAUD_RATE <BAUD RATE TO TRANSMIT DATA>
#define FREQUENCY_TO_PUBLISH_IN_M <FREQUENCY TO PUBLISH IN MINUTES>
#define FREQUENCY_TO_PUBLISH_IN_MS (FREQUENCY_TO_PUBLISH_IN_M * 60 * 1000)
```

| ⚠️ Make sure that the slave device transmits at the same baud rate defined in the BAUD_RATE macro.

## Future changes

We are planning to add the following features in the future:

* Add support for sensors without dependency on an external MCU.
* Receive signals from the MQTT server to control some actuators.
* Set the device to sleep mode to save energy.
* Set the WiFi credentials through the next options:
  * Through the MQTT server.
  * Through a web app.
  * Through a mobile app.
  * Through a Bluetooth connection.

## Libraries

The following libraries are used in this project:

* [Ubidots Arduino MQTT](https://github.com/ubidots/esp32-mqtt)
* [HardwareSerial](https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.h)
* [Arduino](https://www.arduino.cc/reference/en/libraries/)

## References

* [Connect an ESP32-DevKitC to Ubidots over MQTT](https://help.ubidots.com/en/articles/748067-connect-an-esp32-devkitc-to-ubidots-over-mqtt)
* [ESP32 UART Communication Explained with Example](https://microcontrollerslab.com/esp32-uart-communication-pins-example/)
