#ifndef TEMPERATURE_CLIENT_H
#define TEMPERATURE_CLIENT_H

#define MQTT_PUBLISH_TEMPERATURE_INTERVAL 30000 // 30 sec

#include <Arduino.h>
#include <DHT.h>
#include <MqttClient.h>


class TemperatureClient {
public:

  void init(const char *deviceName,
            uint8_t     sensorPin,
            uint8_t     sensorType,
            MqttClient *mqttClient,
            const char *mqttTopic,
            float       correctionTemperature = 0.0);

  void publishStatus(const char *messageId,
                     bool        forcePublish);
  void loop();

private:

  uint8_t sensorPin;
  uint8_t sensorType;
  long lastTemperatureStatusMsgSentAt;
  float correctionTemperature;
  MqttClient *mqttClient;
  const char *mqttTopic;
  byte errosCount = 0;
};

#endif // ifndef TEMPERATURE_CLIENT_H
