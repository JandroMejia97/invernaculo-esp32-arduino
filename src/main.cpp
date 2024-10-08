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
const long tempHumInterval = 60000 * 10; // 1 minuto
const long soilHumInterval = 3000000 * 3; // 5 minutos
const long lightInterval = 3000000 * 3;   // 5 minutos

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
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  message.trim();
  const char* strMessage = message.c_str();
  printf("Mensaje recibido en el topic %s: %s\n", topic, strMessage);
  if (String(topic).indexOf(VARIABLE_LABEL_FAN)) {
    const uint8_t fanValue = atoi(strMessage);
    printf("Valor del ventilador: %d\n", fanValue);
    if (fanValue == 1) {
      fanCurrentState = "ON";
      Serial.println("Ventilador encendido remotamente.");
    } else if (fanValue == 0) {
      fanLastValue = "OFF";
      Serial.println("Ventilador apagado remotamente.");
    }

    if (fanCurrentState != fanLastValue) {
      const float value = fanCurrentState == "ON" ? HIGH : LOW;
      digitalWrite(FAN_PIN, value);
      ubidots.add(VARIABLE_LABEL_FAN, value);
      ubidots.publish(DEVICE_LABEL);
    }

  } else if (String(topic).indexOf(VARIABLE_LABEL_WATER_PUMP)) {
    const uint8_t waterPumpValue = atoi(strMessage);
    printf("Valor de la bomba de agua: %d\n", waterPumpValue);
    if (waterPumpValue == 1) {
      waterPumpCurrentState = "ON";
      Serial.println("Bomba de agua encendida remotamente.");
    } else if (waterPumpValue == 0) {
      waterPumpLastValue = "OFF";
      Serial.println("Bomba de agua apagada remotamente.");
    }

    if (waterPumpCurrentState != waterPumpLastValue) {
      const float value = waterPumpCurrentState == "ON" ? HIGH : LOW;
      digitalWrite(WATER_PUMP_PIN, value);
      ubidots.add(VARIABLE_LABEL_WATER_PUMP, value);
      ubidots.publish(DEVICE_LABEL);
    }
  }
}
