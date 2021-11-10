#include <Ethernet.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

/*****************  START USER CONFIG SECTION *********************************/
#define MQTT_SERVER "YOUR_MQTT_SERVER_ADDRESS"
#define MQTT_PORT 1883
#define MQTT_USERNAME "YOUR_MQTT_USER_NAME"
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"
#define PIN_1 1
#define PIN_2 2

/*****************  END USER CONFIG SECTION *********************************/

EthernetClient ethClient;
PubSubClient client(ethClient);

//Global Variables
bool boot = true;
char charPayload[50];
byte mac[] = {0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED};
const char *mqtt_server = MQTT_SERVER;
const int mqtt_port = MQTT_PORT;
const char *mqtt_user = MQTT_USERNAME;
const char *mqtt_pass = MQTT_PASSWORD;
const int pin_1 = PIN_1;

void reconnect()
{
    int retries = 0;
    while (!client.connected())
    {
        if (retries < 150)
        {
            Serial.print("Attempting MQTT connection...");
            if (client.connect("arduino", mqtt_user, mqtt_pass))
            {
                Serial.println("connected");
                if (boot == false)
                {
                    client.publish("homeassistant/switch/connection", "Reconnected");
                }
                if (boot == true)
                {
                    client.publish("homeassistant/switch/connection", "Rebooted");
                }
                client.subscribe("homeassistant/switch/1");
            }
            else
            {
                Serial.print("failed, rc=");
                Serial.println(client.state());
                retries++;
                delay(5000);
            }
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived: ");
    String newTopic = topic;
    Serial.print(topic);
    payload[length] = '\0';
    String newPayload = String((char *)payload);
    int intPayload = newPayload.toInt();
    Serial.println(newPayload);
    Serial.println();
    newPayload.toCharArray(charPayload, newPayload.length() + 1);

    if (newTopic == "homeassistant/switch/1")
    {
        client.publish("homeassistant/switch/1/state", charPayload);
        digitalWrite(pin_1, HIGH);
        delay(500);
        digitalWrite(pin_1, LOW);
    }
}

void setup()
{
    pinMode(pin_1, OUTPUT);
    Serial.begin(115200);
    if (Ethernet.begin(mac) == 0)
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        for (;;)
            ;
    }
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    delay(10);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}
