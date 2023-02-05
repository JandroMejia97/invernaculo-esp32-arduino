#include "UbidotsEsp32Mqtt.h"
#include <HardwareSerial.h>

// Serial port to communicate with the EDU-CIAA
#define UART 2
// Serial port pins
#define SERIAL_RX 16
#define SERIAL_TX 17

typedef enum {
  ESP_INIT,
  ESP_NO_CONNECTED,
  ESP_CONNECTED,
  ESP_SENDING_DATA,
  ESP_NET_ERROR,
  ESP_UART_ERROR,
} ESP_Status_t;

ESP_Status_t ESP_STATUS = ESP_INIT;

String ESP_GetErrorAsString(ESP_Status_t status) {
  switch (status) {
    case ESP_INIT:
      return "ESP_INIT";
    case ESP_NO_CONNECTED:
      return "ESP_NO_CONNECTED";
    case ESP_CONNECTED:
      return "ESP_CONNECTED";
    case ESP_SENDING_DATA:
      return "ESP_SENDING_DATA";
    case ESP_NET_ERROR:
      return "ESP_NET_ERROR";
    case ESP_UART_ERROR:
      return "ESP_UART_ERROR";
    default:
      return "ESP_UNKNOWN_ERROR";
  }
}

// WiFi credentials
const char *WIFI_SSID = "Fibertel WiFi839 2.4Ghz"; // Enter your WiFi name
const char *WIFI_PASSWORD = "00496026574";  // Enter WiFi password

// Ubidots TOKEN
const char *UBIDOTS_TOKEN = "BBFF-Pc8IvgnCXtKOaQ1lwUfq5oypeSU5AW";
// Device label
const char *DEVICE_LABEL = "esp32-edu-ciaa";
// Variable labels
const char *VARIABLE_LABELS[4] = {"temperature", "soil-humidity", "light", "air-humidity"};

// Message received from the UART
String message;

// Ubidots client
Ubidots client(UBIDOTS_TOKEN);

// Hardware serial port to communicate with the EDU-CIAA
HardwareSerial SerialPort(UART);

/**
 * @brief Callback function to handle the data received from the MQTT broker
 * 
 * @param topic - Topic where the data was published
 * @param payload - Data received
 * @param length - Length of the data received
 */
void callback(char *topic, byte *payload, unsigned int length);

/**
 * @brief Function to handle the data received from the UART
 */
void uartHandler();

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(115200);
  // Set hardware serial baud to 115200;
  SerialPort.begin(115200, SERIAL_8N1, SERIAL_RX, SERIAL_TX);
  // Connecting to a WiFi network
  client.connectToWifi(WIFI_SSID, WIFI_PASSWORD);
  // Connecting to a mqtt broker
  client.setCallback(callback);
  client.setup();
  client.reconnect();
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  int i;
  char data[length];
  for (i = 0; i < length; i++) {
      data[i] = (char) payload[i];
  }
  data[i] = '\0';
  Serial.printf("Data: \n%s\n", data);
  Serial.println("-----------------------");
}

void loop() {
  if (!client.connected()) {
    ESP_STATUS = ESP_NO_CONNECTED;
    client.reconnect();
  }

  while(SerialPort.available()) {
    uartHandler();
  }

  client.loop();
}

void uartHandler() {
  ESP_STATUS = ESP_SENDING_DATA;
  // Read the message from the UART
  message = SerialPort.readString();
  Serial.printf("Message received: %s");
  // Get the type of the variable, and convert it to an integer
  const int index = message[0] - '0';
  // Check if the type is valid, and if the index is in range
  if (index >= 0 && index < 4) {
    // Add the value to the Ubidots client
    client.add(VARIABLE_LABELS[index], message.substring(1).toFloat());
    // Publish the data to the Ubidots MQTT broker
    client.publish(DEVICE_LABEL);
  } else {
    Serial.println("Error: Invalid type, index out of range");
    ESP_STATUS = ESP_UART_ERROR;
  }
}

