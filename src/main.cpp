#include "UbidotsEsp32Mqtt.h"
#include <HardwareSerial.h>
#include "wifi_credentials.h"

// Baud rate to communicate with the EDU-CIAA
#define BAUD_RATE 115200
// Serial port to communicate with the EDU-CIAA
#define DATA_UART 2
// Serial port pins
#define SERIAL_RX 16
#define SERIAL_TX 17

/*
 * Frequency to publish the data to the Ubidots MQTT broker.
 * The data is published every FREQUENCY_TO_PUBLISH minutes, after
 * the last data was published or the last time the data was read from the UART.
 * For testing purposes, you can set this value to 1 (or minor).
 */
#define FREQUENCY_TO_PUBLISH_IN_M 10
#define FREQUENCY_TO_PUBLISH_IN_MS (FREQUENCY_TO_PUBLISH_IN_M * 60 * 1000)

typedef enum {
  ESP_INIT,
  ESP_CONNECTED,
  ESP_DISCONNECTED,
  ESP_READING_DATA,
  ESP_SENDING_DATA,
  ESP_DATA_ERROR,
} ESP_Status_t;

ESP_Status_t ESP_STATUS = ESP_INIT;

static String ESP_GetErrorAsString(ESP_Status_t status) {
  switch (status) {
    case ESP_INIT:
      return "ESP_INIT";
    case ESP_CONNECTED:
      return "ESP_CONNECTED";
    case ESP_DISCONNECTED:
      return "ESP_DISCONNECTED";
    case ESP_READING_DATA:
      return "ESP_READING_DATA";
    case ESP_SENDING_DATA:
      return "ESP_SENDING_DATA";
    case ESP_DATA_ERROR:
      return "ESP_DATA_ERROR";
    default:
      return "ESP_UNKNOWN_ERROR";
  }
}

// WiFi credentials
const char *WIFI_SSID = WIFI_SSID_SECRETS;
const char *WIFI_PASSWORD = WIFI_PASSWORD_SECRETS;

// Ubidots TOKEN
const char *UBIDOTS_TOKEN = UBIDOTS_TOKEN_SECRETS;
// Device label
const char *DEVICE_LABEL = DEVICE_LABEL_SECRETS;
// Variable labels
const char *VARIABLE_LABELS[4] = {"temperature", "air_humidity", "light", "soil_humidity"};

// Message received from the UART
String message;

// Ubidots client
Ubidots client(UBIDOTS_TOKEN);

unsigned long timer;
const int PUBLISH_FREQUENCY_IN_MS = FREQUENCY_TO_PUBLISH_IN_MS;

// Hardware serial port to communicate with the EDU-CIAA
HardwareSerial SerialPort(DATA_UART);

/**
 * @brief Callback function to handle the data received from the MQTT broker
 * 
 * @param topic - Topic where the data was published
 * @param payload - Data received
 * @param length - Length of the data received
 */
void callback(char *topic, byte *payload, unsigned int length);

/**
 * @brief Function to publish the data to the Ubidots MQTT broker
 */
void publishData(void *pvParameters);

/**
 * @brief Function to handle the data received from the UART
 */
void uartHandler();

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(BAUD_RATE);
  // Set hardware serial baud to 115200;
  SerialPort.begin(BAUD_RATE, SERIAL_8N1, SERIAL_RX, SERIAL_TX);
  // Connecting to a WiFi network
  client.connectToWifi(WIFI_SSID, WIFI_PASSWORD);
  // Connecting to a mqtt broker
  client.setDebug(true);
  client.setCallback(callback);
  client.setup();
  client.reconnect();
  timer = millis();

  /* TaskHandle_t publishDataHandle = NULL;

  xTaskCreatePinnedToCore(
    publishData, // Task function.
    "publishData", // name of task.
    10000, // Stack size of task
    NULL, // parameter of the task
    1, // priority of the task
    &publishDataHandle, // Task handle to keep track of created task
    0
  ); // pin task to core 0

  if (publishDataHandle == NULL) {
    Serial.println("Error creating publishData task");
  } else {
    Serial.println("publishData task created");
    vTaskDelete(publishDataHandle);
  } */
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
    ESP_STATUS = ESP_DISCONNECTED;
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
  int i = 0;
  while (SerialPort.available() && SerialPort.peek() != '\n') {
    message += (char) SerialPort.read();
    i++;
  }
  SerialPort.read();
  // Get the type of the variable, and convert it to an integer
  const int index = message[0] - '0';
  // Check if the type is valid, and if the index is in range
  if (index >= 0 && index < 4) {
    ESP_STATUS = ESP_SENDING_DATA;
    const char* variableLabel = VARIABLE_LABELS[index];
    const int value = message.substring(1).toInt();
    // Add the value to the Ubidots client
    client.add(variableLabel, value);
    long diff = millis() - timer;
    diff = diff < 0 ? -diff : diff;
    if (diff > PUBLISH_FREQUENCY_IN_MS) {
      // Publish the data to the Ubidots MQTT broker
      client.publish(DEVICE_LABEL);
      timer = millis();
    }
    ESP_STATUS = ESP_CONNECTED;
  } else {
    Serial.println("Error: Invalid type, index out of range\r\n");
    ESP_STATUS = ESP_DATA_ERROR;
  }
  message = "";
}

void publishData(void *params) {
  const TickType_t xDelay = PUBLISH_FREQUENCY_IN_MS / portTICK_PERIOD_MS;
  while (1) {
    Serial.printf("ESP_STATUS: %s\r\n", ESP_GetErrorAsString(ESP_STATUS).c_str());
    // Publish the data to the Ubidots MQTT broker
    client.publish(DEVICE_LABEL);
    vTaskDelay(xDelay);
  }
}

