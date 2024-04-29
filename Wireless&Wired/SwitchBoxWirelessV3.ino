#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h> 

// WiFi credentials
const char* ssid = "UCLA_Rocket_router";
const char* password = "electronics";

// MQTT Broker details
const char* mqtt_server = "10.9.9.2"; // Change it to your MQTT broker address
const int mqtt_port = 1883;
//const char* mqtt_username = "groundsystems"; // If your broker requires authentication
//const char* mqtt_password = "electronics";

// MQTT topics
const char* switch_topic = "esp32/switch";
const char* data_topic = "esp32/data";

WiFiClient espClient;
PubSubClient client(espClient);

#define RO_PIN 16 //32 boRD
#define DI_PIN 17 //35
//jumper wire to HIGH

#define RE 26 //13
#define DE 27
//RE to 26
//DE to 27

#define purge 22 //22
#define fill 23 //23
#define abortValve 21 //21
#define dump 19 //19
#define vent 18 //18
#define qd 4 //4 // always on
#define ignite 2 //2
#define mpv 15 //15
#define heatpad 5 //17
#define siren 13 //22
#define sirenPower 32 //23

unsigned long long delay_time = 500;
unsigned long long last_time = 0;
String message = "";
//String data = "";
bool wired = false;

HardwareSerial rs485Serial(2);

void setup_wifi() {
  // Connect to Wi-Fi network with SSID and password
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long long disconnect_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - disconnect_time > 15000){
      rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
      Serial.println("wired connection");
      wired = true;
      return;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  //Serial.print("Message arrived [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    if ((i == 0 && (char)payload[i] != 'A') || (i == length - 2 && (char)payload[i] != 'Z')){
        return;
    }

    if (i > 0 && i < length && i != length - 2){
      Serial.print((char)payload[i]);
      //data += (char)payload[i];
    }
  }
  //Serial.println(data);
}

void reconnect() {
  // Loop until we're reconnected
  unsigned long long disconnect_time = millis();
  while (!client.connected()) {
    Serial.println("Unavailable...");
    if (millis() - disconnect_time > 15000){
      rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
      Serial.println("wired connection");
      wired = true;
      return;
    }
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client"/*, mqtt_username, mqtt_password)*/)) {
      Serial.println("connected");
      client.subscribe(data_topic, 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);\
  client.setCallback(callback);


  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(RE, HIGH);
  digitalWrite(DE, HIGH); 

  pinMode(purge, INPUT_PULLDOWN);
  pinMode(fill, INPUT_PULLDOWN);
  pinMode(abortValve, INPUT_PULLDOWN);
  pinMode(dump, INPUT_PULLDOWN);
  pinMode(vent, INPUT_PULLDOWN);
  pinMode(qd, INPUT_PULLDOWN);
  pinMode(ignite, INPUT_PULLDOWN);
  pinMode(mpv, INPUT_PULLDOWN);
  pinMode(siren, INPUT_PULLDOWN);
  pinMode(sirenPower, OUTPUT);

  digitalWrite(sirenPower, HIGH);
  

  Serial.println("Setup Complete");

  last_time = millis();
}

void loop() {
  if (!wired && !client.connected()) {
      reconnect();
  }
  client.loop();
   
  unsigned long currentMillis = millis();
  // Every X number of seconds 
  // it publishes a new MQTT message
  if((millis() - last_time) > delay_time)
  {
    String Purge = String(digitalRead(purge));
    String Fill = String(digitalRead(fill));
    String AbortValve = String(digitalRead(abortValve));
    String Dump = String(digitalRead(dump));
    String Vent = String(digitalRead(vent));
    String QD = String (digitalRead(qd));
    String Ignite = String(digitalRead(ignite));
    String MPV = String(digitalRead(mpv));
    String Heatpad = String(digitalRead(heatpad));
    String Siren = String(digitalRead(siren));

    message = ('A' + AbortValve + QD + Vent + Ignite + Purge + Fill + Dump + Heatpad + MPV + Siren + 'Z');

    if (!wired){

      // Check message length before publishing
      if (message.length() <= MQTT_MAX_PACKET_SIZE) {
        client.publish(switch_topic, message.c_str());
        //Serial.println("Sent:");
        //Serial.println(message);
      } else {
        //Serial.println("Message too long to publish.");
      }

      /*
      if (data != ""){
        //Serial.println("Actuation: ");
        //Serial.println(actuation);
        data = "";
      }*/
    } else {
      rs485Serial.println(message);
      Serial.println("Sent:");
      Serial.println(message);
    }

    message = "";
    last_time = millis();
  }
}
