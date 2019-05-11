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
  this->lastHumidity                   = 0.0;
  this->errosCount                     = 0;
  this->mqttClient                     = mqttClient;
  this->mqttTopic                      = mqttTopic;
  this->correctionTemperature          = correctionTemperature;
}

void TemperatureClient::publishStatus(const char *messageId,
                                      bool        forcePublish) {
  unsigned long now = millis();

  if ((forcePublish) || (now - this->lastTemperatureStatusMsgSentAt >
                         MQTT_PUBLISH_TEMPERATURE_INTERVAL)) {
    if (now < (this->sensorLastReadTime + this->sensorMinDelay)) {
      // It's not time to read the sensor
      PRINTLN_D("TEMPERATURE: It's too early to read the sensor!");
      return;
    }

    DHT_Unified dht(this->sensorPin, this->sensorType);
    sensors_event_t eventTemperature;
    sensors_event_t eventHumidity;
    float humidity    = 0.0;
    float temperature = 0.0;

    dht.begin();
    dht.temperature().getEvent(&eventTemperature);
    dht.humidity().getEvent(&eventHumidity);
    humidity    =  eventHumidity.relative_humidity;
    temperature = eventTemperature.temperature;

    if (isnan(humidity) || isnan(temperature)) {
      PRINTLN_E("TEMPERATURE: Failed to read the sensor!");
      return;
    }

    this->sensorLastReadTime = millis();
    temperature              = temperature + correctionTemperature;

    const size_t bufferSize = 2 * JSON_OBJECT_SIZE(2);
    DynamicJsonDocument root(bufferSize);

    JsonObject status = root.createNestedObject("status");
    status["temperature"] = temperature;
    status["humidity"]    = humidity;
    status["heatIndex"]   = temperature;

    if (messageId != NULL) {
      root["messageId"] = messageId;
    }

    // convert to String
    String outString;
    serializeJson(root, outString);

    this->lastTemperatureStatusMsgSentAt = millis();

    // publish the message
    this->mqttClient->publish(this->mqttTopic, outString);
  }
}

float TemperatureClient::getHumidity() {
  if (millis() < (this->sensorLastReadTime + this->sensorMinDelay)) {
    // It's not time to read the sensor
    return this->lastHumidity;
  }

  DHT_Unified dht(this->sensorPin, this->sensorType);
  sensors_event_t eventHumidity;
  float humidity = 0.0;

  dht.begin();
  dht.humidity().getEvent(&eventHumidity);
  humidity =  eventHumidity.relative_humidity;

  if (!isnan(humidity)) {
    this->sensorLastReadTime = millis();
    this->lastHumidity       = humidity;
    return humidity;
  }

  // return the old value
  return this->lastHumidity;
}

void TemperatureClient::loop() {
  publishStatus(NULL, false);
  RemotePrint::instance()->handle();
}
