#include <Arduino.h>
#include <WiFi.h>

#include "Broker.h"
#include "Credentials.h"
#include "PubSubClient.h"

// definitions
#define SUB_TOPIC "esp/command"
#define PUB_TOPIC "esp/logs"

// Objects
WiFiClient esp_client;
PubSubClient mqtt_client(esp_client);

// Function prototypes
void scanNetworks(void);
void connectToNetwork(void);
void displayConnectionDetails(void);

// Global variables
long last_msg = 0;
char msg[50];
int value = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // WI Fi
  scanNetworks();
  connectToNetwork();
  displayConnectionDetails();

  // mqtt connection
  mqtt_client.setServer(broker.ip, broker.port);
  mqtt_client.setCallback(&mqttCallback);

  // LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!mqtt_client.connected()) {
    mqqtReconnect();
  }
  mqtt_client.loop();

  ///// to do
}

void scanNetworks(void) {
  Serial.println(".................................");
  Serial.println("Scanning available Wi-Fi networks");
  Serial.println(".................................");
  byte num_SSID = WiFi.scanNetworks();
  for (int i = 0; i < num_SSID; i++) {
    Serial.printf("Network # %d", i);
    Serial.println(WiFi.SSID(i));
  }
  Serial.println(".................................");
  Serial.println("Wi-Fi scan complete..............");
  Serial.println(".................................");
}

void connectToNetwork(void) {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to: ");
    Serial.println(credentials.ssid);
    WiFi.begin(credentials.ssid, credentials.password);
    if (WiFi.status == WL_IDLE_STATUS) {
      Serial.println("Device On. Attemping to connect...")
    }
    delay(2000);
  }
  Serial.println("Wi-Fi connected successfuly.......");
}
void displayConnectionDetails(void) {
  long RSSI = WiFi.RSSI();
  IPAddress ip;
  ip = WiFi.localIP();
  Serial.print("Network : ");
  Serial.println(credentials.ssid);
  Serial.print("Device IP Address: ");
  Serial.println(ip);
  Serial.printf("Signal strength: %l dbm", RSSI);
}
void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  if (String(topic) == SUB_TOPIC) {
    Serial.print("Changing output to: ");
    if (messageTemp == "on") {
      Serial.println("ON");
    } else if (messageTemp == "off") {
      Serial.println("OFF");
    }
  }
}
void mqqtReconnect() {
  while (!mqtt_client.connect()) {
    Serial.println("Attempting MQTT connection ......");
    // create random client ID
    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);
    // Attempt reconnect
    if (mqtt_client.connect(clientID.c_str())) {
      Serial.println("connected");

      mqtt_client.publish(PUB_TOPIC, "Hello, world");

      mqtt_client.subscribe(SUB_TOPIC);
    } else {
      Serial.print("failed, rc= ");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}
