#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <MqttClient.h>
#include <RemotePrint.h>
#include <DHT.h>
#include "TemperatureClient.h"


void TemperatureClient::init(const char *deviceName,
                             uint8_t     sensorPin,
                             uint8_t     sensorType,
                             MqttClient *mqttClient,
                             const char *mqttTopic,
                             float       correctionTemperature) {
  this->sensorPin                      = sensorPin;
  this->sensorType                     = sensorType;
  this->lastTemperatureStatusMsgSentAt = 0;
  this->mqttClient                     = mqttClient;
  this->mqttTopic                      = mqttTopic;
  this->correctionTemperature          = correctionTemperature;
}

void TemperatureClient::publishStatus(const char *messageId,
                                      bool        forcePublish) {
  long now = millis();

  if ((forcePublish) or (now - this->lastTemperatureStatusMsgSentAt >
                         MQTT_PUBLISH_TEMPERATURE_INTERVAL)) {
    this->lastTemperatureStatusMsgSentAt = now;

    // Reading temperature or humidity takes about 250 milliseconds!
    DHT dht(this->sensorPin, this->sensorType);

    float humidity = dht.readHumidity();

    // Read temperature as Celsius (the default)
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      this->errosCount++;

      if (this->errosCount >= 10) {
        PRINTLN_E("TEMPERATURE: Failed to read from DHT sensor!");
      }

      // Check if any reads failed and exit early (to try again).
      this->lastTemperatureStatusMsgSentAt = 0;
      return;
    }
    this->errosCount = 0;
    temperature      = temperature + correctionTemperature;

    // Compute heat index in Celsius (isFahreheit = false)
    float heatIndex = dht.computeHeatIndex(temperature, heatIndex, false);

    const size_t bufferSize = JSON_OBJECT_SIZE(3);
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject& root   = jsonBuffer.createObject();
    JsonObject& status = root.createNestedObject("status");
    status["temperature"] = temperature;
    status["humidity"]    = humidity;
    status["heatIndex"]   = heatIndex;

    if (messageId != NULL) {
      root["messageId"] = messageId;
    }

    // convert to String
    String outString;
    root.printTo(outString);

    // publish the message
    this->mqttClient->publish(this->mqttTopic, outString);
  }
}

void TemperatureClient::loop() {
  publishStatus(NULL, false);
  RemotePrint::instance()->handle();
}
