#ifndef TEMPERATURE_CLIENT_H
#define TEMPERATURE_CLIENT_H

#define MQTT_PUBLISH_TEMPERATURE_INTERVAL 30000 // 30 sec
#define PIN_TEMP D3                             // DHT 22 PIN
#define TEMP_SENSOR_TYPE DHT22

#include <Arduino.h>
#include "DHT.h"
#include "MqttClient.h"


class TemperatureClient {
public:

  void init(const char *deviceName,
            uint8_t     sensorPin,
            uint8_t     sensorType,
            MqttClient *mqttClient,
            const char *mqttTopic);

  void publishStatus(const char *messageId,
                     bool        forcePublish);
  void loop();

private:

  uint8_t sensorPin;
  uint8_t sensorType;
  long lastTemperatureStatusMsgSentAt;
  MqttClient *mqttClient;
  const char *mqttTopic;
};

#endif // ifndef TEMPERATURE_CLIENT_H
