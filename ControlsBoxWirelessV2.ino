#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "UCLA_Rocket_Repeat";
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

int state;
String message = "";
String actuation = "";

unsigned long long delay_time = 250;
unsigned long long last_time = 0;  
unsigned long long last_time_send = 0;  

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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {    
    if ((i == 0 && (char)payload[i] != 'A') || (i == length - 1 && (char)payload[i] != 'Z')){
        return;
    }

    if (i > 0 && i < length - 1){
      if ((char)payload[i] != '0' && (char)payload[i] != '1'){
        Serial.println("Message: invalid message");
        return;
      }
      message += (char)payload[i];
    }
  }
  Serial.println("Message: " + message);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32ClientControls"/*, mqtt_username, mqtt_password*/)) {
      Serial.println("connected");
      // Once connected, subscribe to the topic
      client.subscribe(switch_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
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

  last_time = millis();
  last_time_send = millis();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  while (millis() - last_time > 3000){
    Serial.println("Unavailable...");
    if (millis() - last_time > 10000){
      digitalWrite(abortSiren, HIGH);
    }
    if (millis() - last_time > 20000){
      Serial.println("Aborted...");
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
  }
  

  if (message != ""){
    const char ACTUATED = '1';

    const short PURGE_SWITCH = 4;
    const short FILL_SWITCH = 5;
    const short ABORT_SIREN_SWITCH = 9;
    const short DUMP_SWITCH = 6; 
    const short VENT_SWITCH = 2;
    const short QD_SWITCH = 1;
    const short IGNITE_SWITCH = 3;
    const short MPV_SWITCH = 8;
    const short OUTLET_SWITCH = 7;
    const short ABORT_VALVE_SWITCH = 0;

    if(message[FILL_SWITCH] == ACTUATED)
      digitalWrite(fill, LOW);
    if(message[FILL_SWITCH] == '0')
      digitalWrite(fill, HIGH);
    
    if(message[ABORT_SIREN_SWITCH] == ACTUATED)
      digitalWrite(abortSiren, HIGH);
    if(message[ABORT_SIREN_SWITCH] == '0')
      digitalWrite(abortSiren, LOW);
    
    if(message[DUMP_SWITCH] == ACTUATED)
      digitalWrite(dump,LOW);
    if(message[DUMP_SWITCH] == '0')
      digitalWrite(dump, HIGH);
      
    if(message[VENT_SWITCH] == ACTUATED)
      digitalWrite(vent,LOW);
    if(message[VENT_SWITCH] == '0')
      digitalWrite(vent, HIGH);
  
    if(message[PURGE_SWITCH] == ACTUATED)
      digitalWrite(purge, LOW);
    if(message[PURGE_SWITCH] == '0')
      digitalWrite(purge, HIGH);

    if(message[QD_SWITCH] == ACTUATED)
      digitalWrite(qd,LOW);
    if(message[QD_SWITCH] == '0')
      digitalWrite(qd, HIGH);

    if(message[IGNITE_SWITCH] == ACTUATED)
      digitalWrite(ignite, HIGH);
    if(message[IGNITE_SWITCH] == '0')
      digitalWrite(ignite, LOW);

    if(message[MPV_SWITCH] == ACTUATED)
      digitalWrite(mpv, LOW);
    if(message[MPV_SWITCH] == '0')
      digitalWrite(mpv, HIGH);

    if(message[OUTLET_SWITCH] == ACTUATED)
      digitalWrite(outlet, LOW);
    if(message[OUTLET_SWITCH] == '0')
    digitalWrite(outlet, HIGH);

    if(message[ABORT_VALVE_SWITCH] == ACTUATED)
      digitalWrite(abortValve, HIGH);
    if(message[ABORT_VALVE_SWITCH] == '0')
    digitalWrite(abortValve, LOW);
    
    last_time = millis();

    message = "";
  }
  
  if (millis() - last_time_send > delay_time) {
    String abortSirenStr = String(digitalRead(abortSirenAc));
    String igniteStr = String(digitalRead(igniteAc));
    String fillStr = String(digitalRead(fillAc));
    String ventStr = String(digitalRead(ventAc));
    String dumpStr = String(digitalRead(dumpAc));
    String qdStr = String(digitalRead(qdAc));
    String mpvStr = String(digitalRead(mpvAc));
    String purgeStr = String(digitalRead(purgeAc));

    actuation = "A" + abortSirenStr + 
                " " + igniteStr + 
                " " + fillStr + 
                " " + ventStr + 
                " " + dumpStr +
                " " + qdStr +
                " " + mpvStr +
                " " + purgeStr + "Z";
    
    // Check if the message length exceeds the maximum packet size
    /*if (actuation.length() <= MQTT_MAX_PACKET_SIZE) {
        // Publish the message if it's within the size limit
        client.publish(ac_topic, actuation.c_str());
        Serial.println("Message sent: " + actuation);
    } else {
        // Handle the case where the message is too long
        Serial.println("Message too long to publish.");
    }*/
    actuation = "";

    last_time_send = millis();
  }    
}
