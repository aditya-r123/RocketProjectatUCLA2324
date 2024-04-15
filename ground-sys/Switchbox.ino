////////// ARDUINO UNO CODE //////////
/*
#include "SoftwareSerial.h"

#define TX 12
#define RX 13

EspSoftwareSerial::UART rs485Serial;
rs485Serial.enableIntTx(false)

rs485Serial.begin(115200, EspSoftwareSerial::SWSERIAL_8N1, RX, TX)
*/

#include <HardwareSerial.h> 

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

unsigned long long delay_time = 250;
unsigned long long last_time = 0;


HardwareSerial rs485Serial(2);

//SoftwareSerial rs485Serial(RO_PIN, DI_PIN);

void setup() {
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
  
  Serial.begin(115200);
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

  Serial.println("Setup Complete");

  last_time = millis();
}

void loop() {

  if((millis() - last_time) > delay_time)
  {
    String message;

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
    
    rs485Serial.println(message);
    Serial.println("Sent:");
    Serial.println(message);
    
    last_time = millis();
  }
}
