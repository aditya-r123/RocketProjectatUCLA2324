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
#define Extra 4 //32
//#define SER_BUF_SIZE 1024
#define fill 22
#define dump 21 
#define vent 19
#define qd 18
#define purge 5
#define mpv 2
#define ignite 15
#define abortSiren 13

unsigned long long delay_time = 250;
unsigned long long last_time = 0;

HardwareSerial rs485Serial(2);

//SoftwareSerial rs485Serial(RO_PIN, DI_PIN);

void setup() {
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);

  Serial.begin(115200);

  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

  pinMode(abortSiren, OUTPUT);
  pinMode(ignite, OUTPUT);
  pinMode(fill, OUTPUT);
  pinMode(vent, OUTPUT);
  pinMode(dump, OUTPUT);
  pinMode(qd, OUTPUT);
  pinMode(mpv, OUTPUT);  
  pinMode(purge, OUTPUT);  

  digitalWrite(abortSiren, LOW);
  digitalWrite(ignite, LOW);
  digitalWrite(fill, HIGH);
  digitalWrite(vent, HIGH);
  digitalWrite(dump, HIGH);
  digitalWrite(qd, HIGH);
  digitalWrite(mpv, HIGH);
  digitalWrite(purge, HIGH);

  last_time = millis();
}

void loop() {
  String r;
  while (!rs485Serial.available()){
    if (millis() - last_time > 10000){
      digitalWrite(abortSiren, HIGH);
    }
    if (millis() - last_time > 20000){
      digitalWrite(ignite, LOW);
      digitalWrite(fill, HIGH);
      digitalWrite(vent, HIGH);
      digitalWrite(dump, HIGH);
      digitalWrite(qd, HIGH);
      digitalWrite(mpv, HIGH);
      digitalWrite(purge, HIGH);
    }
  }

  if((millis() - last_time) > delay_time)
  {
    while (rs485Serial.available()){
      char received = rs485Serial.read();
      if (received == 'A'){
        for (int i = 0; i < 8; i++){
          
          received = rs485Serial.read();
          r += received;
        }
        Serial.println("Recieved");
        Serial.println(r);
      } 
      const char ACTUATED = '1';

      const short PURGE_SWITCH = 0;
      const short FILL_SWITCH = 1;
      const short ABORT_SIREN_SWITCH = 2;
      const short DUMP_SWITCH = 3; 
      const short VENT_SWITCH = 4;
      const short QD_SWITCH = 5;
      const short IGNITE_SWITCH = 6;
      const short MPV_SWITCH = 7;
      const short ABORT_SWITCH = 8;

      for (int i = 0; i < 9; i++){
        if(r[FILL_SWITCH] == ACTUATED)
          digitalWrite(fill, LOW);
        else
          digitalWrite(fill, HIGH);
          
        if(r[ABORT_SIREN_SWITCH] == ACTUATED)
          digitalWrite(abortSiren, LOW);
        else
          digitalWrite(abortSiren, HIGH);
          
        if(r[DUMP_SWITCH] == ACTUATED)
          digitalWrite(dump,LOW);
        else
          digitalWrite(dump, HIGH);
          
        if(r[VENT_SWITCH] == ACTUATED)
          digitalWrite(vent,LOW);
        else
          digitalWrite(vent, HIGH);
      
        if(r[PURGE_SWITCH] == ACTUATED)
          digitalWrite(purge, LOW);
        else
          digitalWrite(purge, HIGH);

        if(r[QD_SWITCH] == ACTUATED)
          digitalWrite(qd,LOW);
        else
          digitalWrite(qd, HIGH);

        if(r[IGNITE_SWITCH] == ACTUATED) //&& message[QD_SWITCH] == ACTUATED)
          digitalWrite(ignite,LOW);
        else
          digitalWrite(ignite, HIGH);

        if(r[MPV_SWITCH] == ACTUATED)
          digitalWrite(mpv, LOW);
        else
          digitalWrite(mpv, HIGH);

        if(r[ABORT_SWITCH] == ACTUATED)
          digitalWrite(abortSiren, HIGH);
          digitalWrite(ignite, LOW);
          digitalWrite(fill, HIGH);
          digitalWrite(vent, HIGH);
          digitalWrite(dump, HIGH);
          digitalWrite(qd, HIGH);
          digitalWrite(mpv, HIGH);
          digitalWrite(purge, HIGH);
      }  
      last_time = millis();
    }
  }
}
/*
#include <HardwareSerial.h>
//Ethernet connection 
#define RO_PIN 16
#define DI_PIN 17

#define fill 22
#define dump 21 
#define vent 19
#define qd 18
#define purge 5
#define extra 4
#define mpv 2
#define ignite 15
#define abortSiren 13

unsigned long delayStart = 0;
unsigned long current = 0;

HardwareSerial rs485Serial(2);

//SoftwareSerial rs485Serial(RO_PIN, DI_PIN); // RO_PIN for Receive, DI_PIN for Transmit

char getNextState(HardwareSerial& ss)
{
  while (!ss.available())
  {
    current = millis();
    while((current - delayStart) > 10000){
      digitalWrite(abortSiren, LOW);
      while((current - delayStart) > 20000)
      {
          digitalWrite(ignite, HIGH);
          digitalWrite(fill, HIGH);
          digitalWrite(vent, HIGH);
          digitalWrite(dump, HIGH);
          digitalWrite(qd, HIGH);
          digitalWrite(mpv, HIGH);
          digitalWrite(purge, HIGH);
        break;
      }
      break;
    }
  }
  delayStart = current;
  return ss.read();
}

void setup() {
  //Ethernet setup
  Serial.begin(115200);
  rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);

  //Define all controls
  pinMode(abortSiren, OUTPUT);
  pinMode(ignite, OUTPUT);
  pinMode(fill, OUTPUT);
  pinMode(vent, OUTPUT);
  pinMode(dump, OUTPUT);
  pinMode(qd, OUTPUT);
  pinMode(mpv, OUTPUT);  
  pinMode(purge, OUTPUT);  

  digitalWrite(abortSiren, HIGH);
  digitalWrite(ignite, HIGH);
  digitalWrite(fill, HIGH);
  digitalWrite(vent, HIGH);
  digitalWrite(dump, HIGH);
  digitalWrite(qd, HIGH);
  digitalWrite(mpv, HIGH);
  digitalWrite(purge, HIGH);

  //while (getNextState(rs485Serial) != 'A')
    //;

  delayStart = millis();
}

void loop() {
  const short message_length = 8;
  char message[message_length];
  label:
  if (millis() - delayStart > 250){
    
    while (rs485Serial.available() && rs485Serial.read() != 'A')
    {
      Serial.println(rs485Serial.read());
      Serial.println("waiting");
    }
  if (rs485Serial.read() == 'A')
    for (short s = 0; s < message_length; s++)
    {
      char bit_message = rs485Serial.read();
      message[s] = bit_message;
      
    }
    Serial.print("There are the state: ");
    for (short s = 0; s < message_length; s++)
    {
      if (message[s] == 'A' || message[s] == 'Z'){
        goto label;
        
      }
        
      Serial.print(message[s]);
    }
    Serial.print("Usable message");
    Serial.println();
    
    const char ACTUATED = '1';

    const short FILL_SWITCH = 1;
    const short ABORT_SIREN_SWITCH = 2;
    const short DUMP_SWITCH = 3; 
    const short VENT_SWITCH = 4;
    const short PURGE_SWITCH = 5;
    const short QD_SWITCH = 6;
    const short IGNITE_SWITCH = 7;
    const short MPV_SWITCH = 8;

    for (int i = 1; i < 9; i++){
      if(message[FILL_SWITCH] == ACTUATED)
        digitalWrite(fill, LOW);
      else
        digitalWrite(fill, HIGH);
        
      if(message[ABORT_SIREN_SWITCH] == ACTUATED)
        digitalWrite(abortSiren, LOW);
      else
        digitalWrite(abortSiren, HIGH);
        
      if(message[DUMP_SWITCH] == ACTUATED)
        digitalWrite(dump,LOW);
      else
        digitalWrite(dump, HIGH);
        
      if(message[VENT_SWITCH] == ACTUATED)
        digitalWrite(vent,LOW);
      else
        digitalWrite(vent, HIGH);
    
      if(message[PURGE_SWITCH] == ACTUATED)
        digitalWrite(purge, LOW);
      else
        digitalWrite(purge, HIGH);

      if(message[QD_SWITCH] == ACTUATED)
        digitalWrite(qd,LOW);
      else
        digitalWrite(qd, HIGH);

      if(message[IGNITE_SWITCH] == ACTUATED) //&& message[QD_SWITCH] == ACTUATED)
        digitalWrite(ignite,LOW);
      else
        digitalWrite(ignite, HIGH);

      if(message[MPV_SWITCH] == ACTUATED)
        digitalWrite(mpv,LOW);
      else
        digitalWrite(mpv, HIGH);
      delayStart = millis();
    }
  }    
  delayStart = millis();
}
*/
