#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "TPLINK";
const char* password = "1123581321";
const char* mqtt_server = "192.168.1.122";

WiFiClient espClient;
PubSubClient client(espClient);

const int ledPin = 16;
int LED_status = LOW;

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266 Client")) {
      Serial.println("connected");
      // ... and subscribe to topic
      client.subscribe("LED");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);

  if (!root.success()) {
    Serial.println("JSON parsing failed!");
  }

  if (root["led"] == 0) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
  } else if (root["led"] == 1) {
    digitalWrite(ledPin, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.println();
}

void setup(void) {
  Serial.begin(9600);


  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT); 
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
