#include <WiFi.h>
#include <PubSubClient.h>

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
#define outlet 15 //32
//#define SER_BUF_SIZE 1024
#define fill 22
#define dump 21 
#define vent 19
#define qd 18
#define purge 5
#define mpv 4
#define ignite 23
#define abortSiren 27
#define abortValve 13
//#define extra1 15
//#define extra2 14

#define fillAc 36
#define dumpAc 39
#define ventAc 34
#define qdAc 35
#define purgeAc 32
#define mpvAc 33
#define abortSirenAc 25
#define igniteAc 26

const short ACTUATED = 0x1;
short message = 0;
short actuation = 0;
bool aborted = false;

unsigned long long delay_time = 5;
unsigned long long last_time_send = 0;  
unsigned long long last_time = 0;  

int relays[] = {abortValve, qd, vent, ignite, purge, fill, dump, outlet, mpv, abortSiren};

//HardwareSerial rs485Serial(2);

void setup_wifi() {
  delay(10);
  // Connect to Wi-Fi network with SSID and password
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (length == sizeof(short)) {
    /*if (!isDigit((char)payload[0])){
      return;
    }*/    
    memcpy(&message, payload, sizeof(message));
  }
  aborted = false;
  last_time = millis();
  Serial.println(message, BIN);
}

void reconnect() {
  // Loop until we're reconnected
  unsigned long long disconnect_time = millis();
  while (!client.connected()) {
    Serial.println("Unavailable...");
    if (millis() - disconnect_time > 10000){
      digitalWrite(abortSiren, HIGH);
    }
    if (millis() - disconnect_time > 20000){
      Serial.println("Aborted...");
      digitalWrite(ignite, LOW);
      digitalWrite(fill, HIGH);
      digitalWrite(vent, HIGH);
      digitalWrite(dump, HIGH);
      digitalWrite(outlet, HIGH);
      digitalWrite(qd, HIGH);
      digitalWrite(mpv, HIGH);
      digitalWrite(purge, HIGH);
      digitalWrite(ignite, LOW);
      digitalWrite(abortValve, LOW);
    }

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32ClientControls"/*, mqtt_username, mqtt_password*/)) {
      Serial.println("connected");
      disconnect_time = millis();
      // Once connected, subscribe to the topic
      client.subscribe(switch_topic, 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - last_time > 1000){
    Serial.println("No message!");
  }
  if (millis() - last_time > 10000){
    aborted = true;
    digitalWrite(abortSiren, HIGH);
  }
  if (millis() - last_time > 20000){
    Serial.println("Aborted...");
    digitalWrite(ignite, LOW);
    digitalWrite(fill, HIGH);
    digitalWrite(vent, HIGH);
    digitalWrite(dump, HIGH);
    digitalWrite(outlet, HIGH);
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

  if (millis() - last_time_send > delay_time) {
    actuation = digitalRead(abortSirenAc) |
                    (digitalRead(igniteAc) << 1) |
                    (digitalRead(fillAc) << 2) |
                    (digitalRead(ventAc) << 3) |
                    (digitalRead(dumpAc) << 4) |
                    (digitalRead(qdAc) << 5) |
                    (digitalRead(mpvAc) << 6) |
                    (digitalRead(purgeAc) << 7);
    
    // Check if the message length exceeds the maximum packet size
    byte payload[sizeof(actuation)];
    memcpy(payload, &actuation, sizeof(actuation));

    // Publish the payload
    client.publish(ac_topic, payload, sizeof(payload));
    //actuation = actuation | 1 << 8;
    //Serial.println("Sent: ");
    //Serial.println(actuation, BIN);
    actuation = 0;

    last_time_send = millis();
  }
}
