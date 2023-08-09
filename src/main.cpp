#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define LOCK 15
#define CONTACT 12

int s = 0;
long lastRead = 0;

#define MQTT_LOCK_RECIEVE "ESP32/LOCK_COMMAND/Vanh-esp32"
#define MQTT_STATUS_REQUEST "Get Vanh-esp32 status"

#define MQTT_LOCK_PUBLISH "ESP32/LOCK_RESPONSE"

const char* ssid = "V13t4nhtr4n";
const char* password = "v13t4nhtr4n";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connect_to_broker() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_LOCK_RECIEVE);
      client.subscribe(MQTT_STATUS_REQUEST);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
  char status[20] = "\0";
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  Serial.println(length);
  for(int i = 0; i<length; i++)
  {
    status[i] = payload[i];
    Serial.print(status[i]);
  }
  Serial.println();
  Serial.println(status);
  if(String(topic) == MQTT_STATUS_REQUEST)
  {
    Serial.print("Status:");
    Serial.println(s);
    if(s == 0)
    {
      client.publish(MQTT_LOCK_PUBLISH, "Vanh-esp32,OFF");
      client.disconnect();
    } 
    else
    {
      client.publish(MQTT_LOCK_PUBLISH, "Vanh-esp32,ON");
      client.disconnect();
    } 
  }
  if(String(topic) == MQTT_LOCK_RECIEVE)
  {
    if(String(status) == "OFF")
    {
      digitalWrite(LOCK, LOW);
      s = 0;
      delay(500);
      client.publish(MQTT_LOCK_PUBLISH, "Vanh-esp32,OFF");
      client.disconnect();
      Serial.println("LOCKED");
    }
    else if(String(status) == "ON")
    {
      digitalWrite(LOCK, HIGH);
      s = 1;
      delay(500);
      client.publish(MQTT_LOCK_PUBLISH, "Vanh-esp32,ON");
      client.disconnect();
      Serial.println("UNLOCK");
    }
  }
}

void setup() {
	Serial.begin(115200);
  while (!Serial);

  pinMode(LOCK, OUTPUT);
  pinMode(CONTACT, INPUT_PULLUP);
  digitalWrite(LOCK, LOW);
  initWiFi();

  client.setServer(MQTT_SERVER, MQTT_PORT );
  client.setCallback(callback);
  connect_to_broker();
  Serial.println("Start transfer");

  // client.publish("Create device", "");
}

void loop() {
  client.loop();
  if (!client.connected()) {
    connect_to_broker();
  }
  long now = millis();
  // if (now - lastRead > 500) {
  //   lastRead = now;
    int contactState = digitalRead(CONTACT);
    if(contactState == 0 && s == 1)
    {
      Serial.println("LOCK");
      client.publish(MQTT_LOCK_RECIEVE, "OFF");
      s = 0;
    } 
  // }
}