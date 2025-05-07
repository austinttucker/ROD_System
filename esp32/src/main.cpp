#include <WiFi.h>
#include "core_mqtt.h"
#include "core_mqtt_state.h"
#include "core_mqtt_agent.h"

const char *ssid = "your_ssid";
const char *password = "your_password";

#define CLIENT_IDENTIFIER "your-client-id"
#define AWS_IOT_ENDPOINT "your-aws-iot-endpoint.amazonaws.com"
#define TOPIC_NAME "your/topic/name"
#define MQTT_PORT 8883

WiFiClientSecure net;
MQTTContext_t mqttContext;
MQTTAgentContext_t mqttAgentContext;

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    // Set up the secure connection
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Initialize the MQTT context
    MQTT_Init(&mqttContext, &net, MQTT_PORT, CLIENT_IDENTIFIER, AWS_IOT_ENDPOINT);

    // Connect to AWS IoT Core
    if (MQTT_Connect(&mqttContext) == MQTTSuccess)
    {
        Serial.println("Connected to AWS IoT Core");
    }
    else
    {
        Serial.println("Failed to connect to AWS IoT Core");
        while (1)
            ;
    }

    // Publish a message to the topic
    if (MQTT_Publish(&mqttContext, TOPIC_NAME, "{\"message\":\"Hello from ESP32\"}") == MQTTSuccess)
    {
        Serial.println("Message published successfully");
    }
    else
    {
        Serial.println("Failed to publish message");
    }
}

void loop()
{
    // Keep the connection alive
    MQTT_ProcessLoop(&mqttContext, 1000);
}