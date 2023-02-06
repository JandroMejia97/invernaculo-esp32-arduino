#ifndef __UBIDOTS_H__
#define __UBIDOTS_H__

#include "UbidotsEsp32Mqtt.h"

typedef enum {
  ESP_INIT,
  ESP_CONNECTED,
  ESP_DISCONNECTED,
  ESP_READING_DATA,
  ESP_SENDING_DATA,
  ESP_DATA_ERROR,
} ESP_Status_t;

/**
 * @brief Status of the ESP32
 */
void setStatus(ESP_Status_t status);

/**
 * @brief Function to get the status of the ESP32
 * 
 * @return ESP_Status_t - Status of the ESP32
 */
ESP_Status_t getStatus();

/**
 * @brief Function to check if the ESP32 is connected to the Ubidots MQTT broker
 * 
 * @return true - If the ESP32 is connected to the Ubidots MQTT broker
 * @return false - If the ESP32 is not connected to the Ubidots MQTT broker
 */
bool clientIsConnected();

/**
 * @brief Function to reconnect the ESP32 to the Ubidots MQTT broker
 */
void clientReconnect();

/**
 * @brief Client loop
 * 
 */
void clientLoop();

/**
 * @brief Function to publish data to the Ubidots MQTT broker
 * 
 * @param index - Index of the variable to publish
 * @param value - Value of the variable to publish
 */
void publishData(int index, int value);

/**
 * @brief Function to get the status of the ESP32 as a string
 * 
 * @param status - Status of the ESP32
 * @return String - Status of the ESP32 as a string
 */
static String ESP_GetErrorAsString(ESP_Status_t status);

/**
 * @brief Function to initialize the Ubidots client
 */
void ubidotsSetup();



#endif // __UBIDOTS_H__
