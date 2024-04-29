/*

PTs:
Fill, Pneumatics, Tank, Chamber, Cross, Manfold

LCs:
Tank, Engine
*/

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

unsigned long long delay_time = 5;
unsigned long long last_time = 0;
short message = 0;
unsigned long long send_delay = 500;
unsigned long long send_time = 0;

//String data = "";

//HardwareSerial rs485Serial(2);

void setup_wifi() {
  // Connect to Wi-Fi network with SSID and password
  //Serial.println();
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (millis() - send_time > send_delay){
    for (int i = 0; i < length; i++) {
      if (i == 0 && !isDigit((char)payload[i])){
        return;
      }
      Serial.print((char)payload[i]);
    }
    send_time = millis();
    //Serial.println(data);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client"/*, mqtt_username, mqtt_password)*/)) {
      //Serial.println("connected");
      client.subscribe(data_topic, 0);
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");

    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);\
  client.setCallback(callback);

  send_time = millis();

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
  

  //Serial.println("Setup Complete");

  last_time = millis();
}

void loop() {
  //unsigned long long start = millis();
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  // Every X number of seconds 
  // it publishes a new MQTT message
   if((millis() - last_time) > delay_time)
  {
    message = digitalRead(abortValve) |
                    (digitalRead(qd) << 1) |
                    (digitalRead(vent) << 2) |
                    (digitalRead(ignite) << 3) |
                    (digitalRead(purge) << 4) |
                    (digitalRead(fill) << 5) |
                    (digitalRead(dump) << 6) |
                    (digitalRead(heatpad) << 7) |
                    (digitalRead(mpv) << 8) |
                    (digitalRead(siren) << 9) |
                    (1 << 10);

    //message = ('A' + AbortValve + QD + Vent + Ignite + Purge + Fill + Dump + Heatpad + MPV + Siren + 'Z');

    // Check message length before publishing
    byte payload[sizeof(message)];
    memcpy(payload, &message, sizeof(message));

    // Publish the payload
  
    client.publish(switch_topic, payload, sizeof(payload));
    //client.publish(switch_topic, message);
    //Serial.println("Sent:");
    //Serial.println(message, BIN);

    message = 0; // Clear the message variable
    //Serial.println(millis() - start);
    last_time = millis();
  }
}
