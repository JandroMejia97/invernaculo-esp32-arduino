# ESP-32s Project with Arduino and PlatformIO

This project consists of an ESP-32s that uses Arduino libraries to receive data (temperature, air/soil humidity, and light intensity) through UART and send it to an MQTT server on Ubidots. PlatformIO is used as the integrated development environment (IDE).

## Before start...
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
