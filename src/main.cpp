#include <WiFi.h>
#include <UbidotsEsp32Mqtt.h>
#include <DHT.h>
#include "wifi_credentials.h"

const char *WIFINAME = WIFI_SSID_SECRETS;
const char *WIFIPASS = WIFI_PASSWORD_SECRETS;
const char *TOKEN = UBIDOTS_TOKEN_SECRETS;
const char *DEVICE_LABEL = DEVICE_LABEL_SECRETS;

// Configuración de MQTT y Ubidots
const char* VARIABLE_LABEL_TEMP = "temperature";
const char* VARIABLE_LABEL_AIR_HUM = "air_humidity";
const char* VARIABLE_LABEL_LIGHT = "light";
const char* VARIABLE_LABEL_SOIL_HUM = "soil_humidity";
const char* VARIABLE_LABEL_FAN = "fan";
const char* VARIABLE_LABEL_WATER_PUMP = "water_pump";

Ubidots ubidots(TOKEN);

// Configuración de sensores
#define DHTPIN 4       // Pin donde está conectado el DHT11
#define DHTTYPE DHT11  // Tipo de sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_MOISTURE_PIN 34 // Pin analógico para humedad del suelo
#define LIGHT_SENSOR_PIN 35  // Pin analógico para la fotoresistencia

// Configuración de actuadores
#define FAN_PIN 12
#define WATER_PUMP_PIN 14

// Variables de muestreo
const long tempHumInterval = 60000; // 1 minuto
const long soilHumInterval = 300000; // 5 minutos
const long lightInterval = 150000; // 2.5 minutos

long lastTempHumTime = 0;
long lastSoilHumTime = 0;
long lastLightTime = 0;
String fanLastValue = "OFF";
String fanCurrentState = "OFF";
String waterPumpLastValue = "OFF";
String waterPumpCurrentState = "OFF";

TaskHandle_t TaskTempHumHandle;
TaskHandle_t TaskSoilHumHandle;
TaskHandle_t TaskLightHandle;

SemaphoreHandle_t sensorMutex;

void setup_ubidots();

void readTempHum(void * parameter);

void readSoilHum(void * parameter);

void readLight(void * parameter);

void controlActuators(float temperature, float airHumidity);

void controlSoilHum(int soilMoisture);

void updateActuatorState(const char* actuatorName, const char* variableLabel, int pin, String* currentState, String* lastState, const char* strMessage);

void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(115200);
  
  pinMode(FAN_PIN, OUTPUT);
  pinMode(WATER_PUMP_PIN, OUTPUT);

  // Iniciar Ubidots
  setup_ubidots();

  // Iniciar el sensor DHT11  
  dht.begin();

  sensorMutex = xSemaphoreCreateMutex();

  // Crear tareas para los sensores, corriendo en el segundo núcleo del ESP32
  xTaskCreatePinnedToCore(readTempHum, "TaskTempHum", 10000, NULL, 1, &TaskTempHumHandle, 1);
  xTaskCreatePinnedToCore(readSoilHum, "TaskSoilHum", 10000, NULL, 1, &TaskSoilHumHandle, 1);
  xTaskCreatePinnedToCore(readLight, "TaskLight", 10000, NULL, 1, &TaskLightHandle, 1);

  Serial.println("Sistema listo.");
}

void setup_ubidots() {
  Serial.print("Conectando a la red: ");
  Serial.println(WIFINAME);

  ubidots.connectToWifi(WIFINAME, WIFIPASS);

  ubidots.setDebug(true);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_FAN);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_WATER_PUMP);

}

void loop() {
  // En el loop principal solo se procesa MQTT para ahorrar energía
  if (!ubidots.connected()) {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_FAN);
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_WATER_PUMP);
  }
  ubidots.loop();
  delay(1000);
}

void readTempHum(void * parameter) {
  for (;;) {
    long now = millis();
    if (now - lastTempHumTime > tempHumInterval && xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      lastTempHumTime = now;
      float temperature = dht.readTemperature();
      float airHumidity = dht.readHumidity();
      xSemaphoreGive(sensorMutex);
      if (!isnan(temperature) && !isnan(airHumidity)) {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C, Air Humidity: ");
        Serial.print(airHumidity);
        Serial.println(" %");

        ubidots.add(VARIABLE_LABEL_TEMP, temperature);
        ubidots.add(VARIABLE_LABEL_AIR_HUM, airHumidity);
        ubidots.publish(DEVICE_LABEL);
        
        controlActuators(temperature, airHumidity);
      } else {
        Serial.println("Error leyendo el DHT11");
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Tiempo de espera para la tarea
  }
}

void readSoilHum(void * parameter) {
  for (;;) {
    long now = millis();
    if (now - lastSoilHumTime > soilHumInterval && xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      lastSoilHumTime = now;
      int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
      const int soilMoistureMapped = map(soilMoisture, 0, 4095, 0, 100);
      xSemaphoreGive(sensorMutex);

      Serial.print("Soil Moisture: ");
      Serial.println(soilMoistureMapped);

      ubidots.add(VARIABLE_LABEL_SOIL_HUM, soilMoistureMapped);
      ubidots.publish(DEVICE_LABEL);
      
      controlSoilHum(soilMoistureMapped);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Tiempo de espera para la tarea
  }
}

void readLight(void * parameter) {
  for (;;) {
    long now = millis();
    if (now - lastLightTime > lightInterval && xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      lastLightTime = now;
      int lightLevel = analogRead(LIGHT_SENSOR_PIN);
      const int lightLevelMapped = map(lightLevel, 0, 4095, 0, 100);
      xSemaphoreGive(sensorMutex);

      Serial.print("Light Level: ");
      Serial.println(lightLevelMapped);

      ubidots.add(VARIABLE_LABEL_LIGHT, lightLevelMapped);
      ubidots.publish(DEVICE_LABEL);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Tiempo de espera para la tarea
  }
}

void controlActuators(float temperature, float airHumidity) {
  if (temperature > 30) {
    fanCurrentState = "ON";
  } else {
    fanCurrentState = "OFF";
  }
  
  if (fanCurrentState != fanLastValue) {
    const float value = fanCurrentState == "ON" ? HIGH : LOW;
    digitalWrite(FAN_PIN, value);
    ubidots.add(VARIABLE_LABEL_FAN, value);
    ubidots.publish(DEVICE_LABEL);
  }
}

void controlSoilHum(int soilMoisture) {
  if (soilMoisture < 20) {
    waterPumpCurrentState = "ON";
  } else {
    waterPumpCurrentState = "OFF";
  }

  if (waterPumpCurrentState != waterPumpLastValue) {
    const float value = waterPumpCurrentState == "ON" ? HIGH : LOW;
    digitalWrite(WATER_PUMP_PIN, value);
    ubidots.add(VARIABLE_LABEL_WATER_PUMP, value);
    ubidots.publish(DEVICE_LABEL);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Build the message string from the payload bytes.
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  const char* strMessage = message.c_str();
  
  // Print received topic and message for debugging.
  printf("Message received on topic %s: %s\n", topic, strMessage);

  // Dispatch to the appropriate handler based on the topic.
  String topicStr(topic);
  if (topicStr.indexOf(VARIABLE_LABEL_FAN) != -1) {
    updateActuatorState("Fan", VARIABLE_LABEL_FAN, FAN_PIN, &fanCurrentState, &fanLastValue, strMessage);
  } else if (topicStr.indexOf(VARIABLE_LABEL_WATER_PUMP) != -1) {
    updateActuatorState("Water Pump", VARIABLE_LABEL_WATER_PUMP, WATER_PUMP_PIN, &waterPumpCurrentState, &waterPumpLastValue, strMessage);
  }
}

void updateActuatorState(const char* actuatorName, const char* variableLabel, int pin, String* currentState, String* lastState, const char* strMessage) {
  // Convert the received message to an integer value.
  const uint8_t actuatorValue = atoi(strMessage);
  printf("%s value received: %d\n", actuatorName, actuatorValue);

  // Set the actuator's current state based on the received value.
  if (actuatorValue == 1) {
    *currentState = "ON";
    Serial.printf("%s turned ON remotely.\n", actuatorName);
  } else if (actuatorValue == 0) {
    *currentState = "OFF";
    Serial.printf("%s turned OFF remotely.\n", actuatorName);
  }

  // Update the actuator only if there is a change in state.
  if (*currentState != *lastState) {
    const float digitalValue = (*currentState == "ON") ? HIGH : LOW;
    digitalWrite(pin, digitalValue);
    ubidots.add(variableLabel, digitalValue);
    ubidots.publish(DEVICE_LABEL);
    *lastState = *currentState;
  }
}
