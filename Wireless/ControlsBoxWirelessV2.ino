#include <WiFi.h>
#include <PubSubClient.h>

// DEBUGGING

// COMMENT FOR OPS (NO DEBUGGING)
#define IF_DEBUG true

#ifdef IF_DEBUG
#define Debug Debug
#define Debugln Debugln

#ifndef IF_DEBUG
#define Debug doNot
#define Debugln doNot
#endif

// Null function
void doNot(char *cstr = "") {}

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
const char* ac_topic = "esp32/ac";

WiFiClient espClient;
PubSubClient client(espClient);

#define RO_PIN 16
#define DI_PIN 17
#define outlet 2 //32
//#define SER_BUF_SIZE 1024
#define fill 22
#define dump 21 
#define vent 19
#define qd 18
#define purge 5
#define mpv 4
#define ignite 15
#define abortSiren 23
#define abortValve 13

#define fillAc 34
#define dumpAc 35
#define ventAc 32
#define qdAc 33
#define purgeAc 25
#define mpvAc 26
#define abortSirenAc 27
#define igniteAc 14

const short ACTUATED = 0x1;
short message = 0;
short actuation = 0;
bool aborted = false;

unsigned long long delay_time = 100;
unsigned long long last_time_send = 0;  
unsigned long long last_time = 0;  

int relays[] = {abortValve, qd, vent, ignite, purge, fill, dump, outlet, mpv, abortSiren};

//HardwareSerial rs485Serial(2);

void setup_wifi() {
  delay(10);
  // Connect to Wi-Fi network with SSID and password
  Debugln();
  Debug("Connecting to ");
  Debugln(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Debug(".");
  }

  Debugln("");
  Debugln("WiFi connected");
  Debugln("IP address: ");
  Debugln(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (length == sizeof(short)) {

    memcpy(&message, payload, sizeof(message));
    
    // Now 'receivedValue' contains the reconstructed short value
  }
  aborted = false;
  last_time = millis();
  Debugln(message, BIN);
}

void reconnect() {
  // Loop until we're reconnected
  unsigned long long disconnect_time = millis();
  while (!client.connected()) {
    Debugln("Unavailable...");
    if (millis() - disconnect_time > 10000){
      digitalWrite(abortSiren, HIGH);
    }
    if (millis() - disconnect_time > 20000){
      Debugln("Aborted...");
      digitalWrite(ignite, LOW);
      digitalWrite(fill, HIGH);
      digitalWrite(vent, HIGH);
      digitalWrite(dump, HIGH);
      digitalWrite(qd, HIGH);
      digitalWrite(mpv, HIGH);
      digitalWrite(purge, HIGH);
      digitalWrite(ignite, LOW);
      digitalWrite(abortValve, LOW);
    }

    Debug("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32ClientControls"/*, mqtt_username, mqtt_password*/)) {
      Debugln("connected");
      disconnect_time = millis();
      // Once connected, subscribe to the topic
      client.subscribe(switch_topic, 0);
    } else {
      Debug("failed, rc=");
      Debug(client.state());
      
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //Define all controls
//  pinMode(spare, OUTPUT);
  pinMode(abortSiren, OUTPUT);
  pinMode(ignite, OUTPUT);
  pinMode(fill, OUTPUT);
  pinMode(vent, OUTPUT);
  pinMode(dump, OUTPUT);
  pinMode(qd, OUTPUT);
  pinMode(mpv, OUTPUT);  
  pinMode(purge, OUTPUT);   
  pinMode(outlet, OUTPUT);
  pinMode(abortValve, OUTPUT);   

  digitalWrite(abortSiren, LOW);//off
  digitalWrite(ignite, LOW);//off
  digitalWrite(fill, HIGH);//closed
  digitalWrite(vent, HIGH);//open
  digitalWrite(dump, HIGH);//open
  digitalWrite(qd, HIGH);//open
  digitalWrite(mpv, HIGH);
  digitalWrite(purge, HIGH);//closed
  digitalWrite(outlet, HIGH);//closed
  digitalWrite(abortValve, LOW);//closed
  
  pinMode(purgeAc, INPUT_PULLDOWN);
  pinMode(fillAc, INPUT_PULLDOWN);
  pinMode(abortSirenAc, INPUT_PULLDOWN);
  pinMode(dumpAc, INPUT_PULLDOWN);
  pinMode(ventAc, INPUT_PULLDOWN);
  pinMode(qdAc, INPUT_PULLDOWN);
  pinMode(igniteAc, INPUT_PULLDOWN);
  pinMode(mpvAc, INPUT_PULLDOWN);

  last_time_send = millis();
  last_time = millis();
}

void loop() {
  // unsigned long long start = millis();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - last_time > 1000){
    Debugln("No message!");
  }
  if (millis() - last_time > 10000){
    aborted = true;
    digitalWrite(abortSiren, HIGH);
  }
  if (millis() - last_time > 20000){
    Debugln("Aborted...");
    digitalWrite(ignite, LOW);
    digitalWrite(fill, HIGH);
    digitalWrite(vent, HIGH);
    digitalWrite(dump, HIGH);
    digitalWrite(qd, HIGH);
    digitalWrite(mpv, HIGH);
    digitalWrite(purge, HIGH);
    digitalWrite(ignite, LOW);
    digitalWrite(abortValve, LOW);
  }

  if (!aborted){
    for (int i = 0; i < 10; i++){
      bool bitValue = (message >> i) & ACTUATED;
      if (i == 0 | i == 3 | i == 9){
        if (bitValue)
          digitalWrite(relays[i], HIGH);
        if (!bitValue)
          digitalWrite(relays[i], LOW);
      }
      else {
        if (bitValue) 
          digitalWrite(relays[i], LOW);
        if (!bitValue)
          digitalWrite(relays[i], HIGH);
      }
    }
  }

  /*if (millis() - last_time_send > delay_time) {
    actuation = digitalRead(abortSirenAc) |
                    (digitalRead(igniteAc) << 1) |
                    (digitalRead(fillAc) << 2) |
                    (digitalRead(ventAc) << 3) |
                    (digitalRead(dumpAc) << 4) |
                    (digitalRead(qdAc) << 5) |
                    (digitalRead(mpvAc) << 6) |
                    (digitalRead(purgeAc) << 7);*/
    
    // Check if the message length exceeds the maximum packet size
    /*byte payload[sizeof(actuation)];
    memcpy(payload, &actuation, sizeof(actuation));

    // Publish the payload
    client.publish("topic", payload, sizeof(payload));
    actuation = 0;*/
     // Debugln(millis() - start);
    last_time_send = millis();
  // }    
}
