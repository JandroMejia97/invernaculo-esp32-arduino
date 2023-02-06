#include "ubidots.h"
#include "wifi_credentials.h"

#if defined WIFI_SSID_SECRETS 
  const char* WIFI_SSID = WIFI_SSID_SECRETS;
#else
  #error "WIFI_SSID not defined. Please define it before importing Ubidots library"
#endif

#if defined WIFI_PASSWORD_SECRETS
  const char* WIFI_PASSWORD = WIFI_PASSWORD_SECRETS;
#else
  #error "WIFI_PASSWORD not defined. Please define it before importing Ubidots library"
#endif

#if defined UBIDOTS_TOKEN_SECRETS
  const char* UBIDOTS_TOKEN = UBIDOTS_TOKEN_SECRETS;
#else
  #error "UBIDOTS_TOKEN not defined. Please define it before importing Ubidots library"
#endif

// Set the DEVICE_LABEL
#ifndef DEVICE_LABEL
  #warning "DEVICE_LABEL not defined. Using default value"
  #define DEFAULT_DEVICE_LABEL "esp32-edu-ciaa"
  const char *DEVICE_LABEL = DEFAULT_DEVICE_LABEL;
#else
  const char *DEVICE_LABEL = DEVICE_LABEL;
#endif

// Variable labels
#ifndef VARIABLE_LABELS
  #warning "VARIABLE_LABELS not defined. Using default value"
  #define DEFAULT_VARIABLE_LABELS {"temperature", "soil-humidity", "light", "air-humidity"}
  const char *LABELS[4] = DEFAULT_VARIABLE_LABELS;
#else
  const char *LABELS[4] = VARIABLE_LABELS;
#endif

/**
 * @brief Status of the ESP32
 */
ESP_Status_t ESP_STATUS = ESP_INIT;

void setStatus(ESP_Status_t status) {
  ESP_STATUS = status;
}

ESP_Status_t getStatus() {
  return ESP_STATUS;
}

/**
 * @brief Public Ubidots client
 */
static Ubidots client(UBIDOTS_TOKEN);
static unsigned long timer;
static const int PUBLISH_FREQUENCY = 15000;

bool clientIsConnected() {
  return client.connected();
}

void clientReconnect() {
  setStatus(ESP_DISCONNECTED);
  client.reconnect();
}

void clientLoop() {
  client.loop();
}

/**
 * @brief Callback function to handle the data received from the MQTT broker
 * 
 * @param topic - Topic where the data was published
 * @param payload - Data received
 * @param length - Length of the data received
 */
void callback(char *topic, byte *payload, unsigned int length);

void ubidotsSetup() {
  // Connecting to a WiFi network
  client.connectToWifi(WIFI_SSID, WIFI_PASSWORD);
  // Set the debug mode
  client.setDebug(true);
  // Connecting to a mqtt broker
  client.setCallback(callback);
  client.setup();
  client.reconnect();
  setStatus(ESP_CONNECTED);
  timer = millis();
}

void publishData(int index, int value) {
  // Check if the type is valid, and if the index is in range
  const int len = sizeof(LABELS) / sizeof(LABELS[0]);
  if (index >= 0 && index < len) {
    ESP_STATUS = ESP_SENDING_DATA;
    const char* variableLabel = LABELS[index];
    // Add the value to the Ubidots client
    client.add(variableLabel, value);

    // Check if the time to publish the data has passed
    long diff = millis() - timer;
    diff = diff < 0 ? -diff : diff;
    if (diff > PUBLISH_FREQUENCY) {
      // Publish the data to the Ubidots MQTT broker
      client.publish(DEVICE_LABEL);
      timer = millis();
    }
    setStatus(ESP_CONNECTED);
  } else {
    Serial.println("Error: Invalid type, index out of range\r\n");
    setStatus(ESP_DATA_ERROR);
  }
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