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

#define RO_PIN 16
#define DI_PIN 17
//jumper wire to HIGH

//#define DE_RE_PIN 35 //32

#define purge 22
#define fill 23
#define abortSiren 21
#define dump 19
#define vent 18
#define qd 4
#define ignite 2
#define mpv 15
#define abort 5
//jumper wire from 17 to 5

unsigned long long delay_time = 250;
unsigned long long last_time = 0;

HardwareSerial rs485Serial(2);

//SoftwareSerial rs485Serial(RO_PIN, DI_PIN);

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);
  
  pinMode(purge, INPUT_PULLDOWN);
  pinMode(fill, INPUT_PULLDOWN);
  pinMode(abortSiren, INPUT_PULLDOWN);
  pinMode(dump, INPUT_PULLDOWN);
  pinMode(vent, INPUT_PULLDOWN);
  pinMode(qd, INPUT_PULLDOWN);
  pinMode(ignite, INPUT_PULLDOWN);
  pinMode(mpv, INPUT_PULLDOWN);
  
  Serial.begin(115200);
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

  last_time = millis();
}

void loop() {

  if((millis() - last_time) > delay_time)
  {

    String Purge = String(digitalRead(purge));
    String Fill = String(digitalRead(fill));
    String AbortSiren = String(digitalRead(abortSiren));
    String Dump = String(digitalRead(dump));
    String Vent = String(digitalRead(vent));
    String QD = String (digitalRead(qd));
    String Ignite = String(digitalRead(ignite));
    String MPV = String(digitalRead(mpv));
    String Abort = String(digitalRead(abort));

    String message = ('A' + Purge + Fill + AbortSiren + Dump + Vent + QD + Ignite + MPV + Abort + 'Z');
    Serial.println(message);
    
    rs485Serial.print(message);
    
    last_time = millis();
  }
}