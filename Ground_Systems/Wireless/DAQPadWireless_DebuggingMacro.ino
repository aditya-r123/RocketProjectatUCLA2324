
#include <RP_ADS1256.h>
#include <HX711.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <WiFi.h>
#include <PubSubClient.h>


/********** DEBUGGING MACRO **********/

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

/*************************************/


// WiFi credentials
const char* ssid = "UCLA_Rocket_router";
const char* password = "electronics";

// MQTT Broker details
const char* mqtt_server = "10.9.9.2"; // Change it to your MQTT broker address
const int mqtt_port = 1883;
//const char* mqtt_username = "groundsystems"; // If your broker requires authentication
//const char* mqtt_password = "electronics";

// MQTT topics
const char* data_topic = "esp32/data";
const char* ac_topic = "esp32/ac";

WiFiClient espClient;
PubSubClient client(espClient);

const int maxChar = 200;
char actuation[11];
char printStr[maxChar];
char storeStr[maxChar];


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
    short recieved;
    memcpy(&recieved, payload, sizeof(recieved));

    for (int i = 0; i < 9; i++){
      actuation[i] = (char)(((recieved >> i) & 1) + '0');
    }
    actuation[9] = ',';
    actuation[10] = ' ';
    // Now 'receivedValue' contains the reconstructed short value
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Debug("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32ClientDAQ"/*, mqtt_username, mqtt_password)*/)) {
      Debugln("connected");
      //client.subscribe(ac_topic);
    } else {
      Debug("failed, rc=");
      Debug(client.state());
      Debugln(" try again in 5 seconds");

    }
  }
}

// GLOBAL CONSTANTS
const int NUM_PT = 6;
const int NUM_LC = 2;
const int NUM_TC = 0;

/*
#include <Adafruit_MAX31856.h>
#include <SPI.h>
#include <Wire.h>*/


// ETHERNET CONNECTION
#define RO_PIN 16
#define DI_PIN 17
#define DE_RE_PIN 23

HardwareSerial rs485Serial(2);


// PRESSURE MEASUREMENT
#define ADS1256_SCK 27
#define ADS1256_MISO 36
#define ADS1256_MOSI 14
#define ADS1256_CS 13
#define ADS1256_DRDY 15

ADS1256 PTADC(ADS1256_DRDY, 0, 0, ADS1256_CS, 2.500); //DRDY, RESET, SYNC(PDWN), CS, VREF(float). 


// FORCE MEASUREMENT
#define LC1_DOUT 32  // Tension Load Cell
#define LC1_SCK 33
#define LC2_DOUT 25  // Tension Load Cell
#define LC2_SCK 26

HX711 LC1;
HX711 LC2;

// LC calibration factors
const float LC1_calibration = 995;    // Pulling is positive, pushing is negative
const float LC2_calibration = 2105;   // Pulling is positive, pushing is negative


// SD CARD READER
#define SDCARD_SCK_PIN 19
#define SDCARD_MISO_PIN 18
#define SDCARD_MOSI_PIN 5
#define SDCARD_SS_PIN 4

String sd_fileName = "/data.csv";

// Functions prototypes for file read/write
void readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);

/*
// IO Expander (TO CHANGE)
#define IOEXPANDER_CSPIN 3
#define dump 4
#define vent 5
#define qd 6
#define mpv 7

Adafruit_MCP23X17 mcp;
*/

/*
// Temperature Measurement (CHANGE CS NUMBER)
// To use software SPI, we need to pass in CS, DI, DO, CLK
Adafruit_MAX31856 tc1 = Adafruit_MAX31856(??, ??, ??, ??); // Type T TC
Adafruit_MAX31856 tc2 = Adafruit_MAX31856(??, ??, ??, ??); // Type K TC
*/

// Data Sending Interval Settings
unsigned long long delay_time = 5;
unsigned long long last_time = 0;

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);\
  //client.setCallback(callback);

  // ETHERNET CONNECTION
  pinMode(DE_RE_PIN, OUTPUT);
  digitalWrite(DE_RE_PIN, HIGH);

  //rs485Serial.begin(115200, SERIAL_8N1, RO_PIN, DI_PIN);
  Serial.begin(115200);


  // PRESSURE MEASUREMENT
  // Initialize SPI object for ADS SPI communication
  SPIClass* ADS_SPI = new SPIClass(VSPI);
  ADS_SPI->begin(ADS1256_SCK, ADS1256_MISO, ADS1256_MOSI, ADS1256_CS);

  // Initialize ADS1256
  PTADC.InitializeADC(ADS_SPI);


  // FORCE MEASUREMENT
  LC1.begin(LC1_DOUT, LC1_SCK);
  LC1.set_scale(LC1_calibration);
  LC1.tare(); // Reset load cell to 0

  LC2.begin(LC2_DOUT, LC2_SCK);
  LC2.set_scale(LC2_calibration);
  LC2.tare(); // Reset load cell to 0


  // SD CARD READER
  SPIClass* SD_SPI = new SPIClass(HSPI);
  SD_SPI->begin(SDCARD_SCK_PIN, SDCARD_MISO_PIN, SDCARD_MOSI_PIN);

  if(!SD.begin(SDCARD_SS_PIN, *SD_SPI)){
    Debugln("Card Mount Failed");
    return;
  }

  // Construct header row for csv file
  std::string dataHeader;
  for(int i=0; i<NUM_PT; i++) dataHeader += "P" + std::to_string(i+1) + ",";
  for(int i=0; i<NUM_LC; i++) dataHeader += "L" + std::to_string(i+1) + ",";
  for(int i=0; i<NUM_TC; i++) dataHeader += "T" + std::to_string(i+1) + (i==NUM_TC-1 ? "\n": ",");
  writeFile(SD, sd_fileName.c_str(), dataHeader.c_str());


  /*
  // IO EXPANDER (TO CHANGE)
  Adafruit_MCP23X17 mcp;
  if (!mcp.begin_SPI(IOEXPANDER_CSPIN)) {
    Debugln("IO Expander Initialization Error");
    while (1);
  }
  */

  /*
  pinMode(fill, INPUT);
  pinMode(dump, INPUT);
  pinMode(vent, INPUT);
  pinMode(qd, INPUT);
  pinMode(mpv, INPUT);
  */

  /*
  // TEMPERATURE MEASUREMENT
  // Initialize TCs
  if (!tc1.begin()) {
    Debugln("Could not initialize tc1.");
    while (1) delay(10);
  }
  if (!tc2.begin()) {
    Debugln("Could not initialize tc2.");
    while (1) delay(10);
  }
  // Set type
  tc1.setThermocoupleType(MAX31856_TCTYPE_T);
  tc2.setThermocoupleType(MAX31856_TCTYPE_K);
  // Set mode
  tc1.setConversionMode(MAX31856_CONTINUOUS);
  tc2.setConversionMode(MAX31856_CONTINUOUS);
  */

  last_time = millis();
}

void loop() 
{
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if((millis() - last_time) > delay_time)
  {
    //if (strlen(actuation) != 0){
      //strcat(printStr, actuation);
      //strcat(storeStr, actuation);
    //}

      // Create variables for all sensors
    float ptVals[6], lcVals[2], tcVals[2]; // TC1: Type T TC, TC2: Type K TC
    sprintf(printStr, "%llu ", millis());
    sprintf(storeStr, "%llu,", millis());

    // Read Pressure Transducer values
    for (int i = 0; i < 8; i++)
    {
      long rawConversion = PTADC.cycleSingle();
      float voltageValue = PTADC.convertToVoltage(rawConversion);

      if(i >= NUM_PT) continue;
      ptVals[i] = voltageValue;
    }

    // Read Load Cell values
    lcVals[0] = LC1.get_units();
    lcVals[1] = LC2.get_units();
    
    /*
    // Read Thermocouple values
    tc1Val = tc1.readThermocoupleTemperature();
    tc2Val = tc2.readThermocoupleTemperature();
    */

    // IO Expander
    
  
    // Calibration for PTs (likely have to calibrate everytime you flow)
    ptVals[0] = ptVals[0] * 425 - 123;
    ptVals[1] = ptVals[1] * 323 - 91.6;
    ptVals[2] = ptVals[2] * 426 - 125;
    ptVals[3] = ptVals[3] * 325 - 90.5;
    ptVals[4] = ptVals[4] * 421 - 126;
    ptVals[5] = ptVals[5] * 425 - 107;
  
    // Sending over Ethernet cable (convert all data to Strings)
    // digitalWrite(DE_RE_PIN, HIGH);
    // delay(1);

    
    /*
    String pt1vals, pt2vals, pt3vals, pt4vals, pt5vals, pt6vals, lc1vals, lc2vals, lc3vals, tc1vals, tc2vals, connectionStatuss;
    String fillAcc, dumpAcc, ventAcc, ignite1Acc, qdAcc, mpvAcc, ignite2Acc, ignite3Acc;
    */

    
    for(int i = 0; i < NUM_PT; i++)
    {
      sprintf(printStr + strlen(printStr), "PT%d: %.2f, ", i + 1, ptVals[i]);
      sprintf(storeStr + strlen(storeStr), "%.2f,", ptVals[i]);
    }
    for(int i = 0; i < NUM_LC; i++)
    {
      sprintf(printStr + strlen(printStr), "LC%d: %.2f,", i + 1, lcVals[i]);
      sprintf(storeStr + strlen(storeStr), "%.2f,", lcVals[i]);
    }

    // Printing to Serial
    //Debugln(printStr.c_str());

    // Sending via Ethernet Cable
    //digitalWrite(DE_RE_PIN, HIGH);
    //delay(50);
      strcat(printStr, "\n");
      if (strlen(printStr) <= MQTT_MAX_PACKET_SIZE) {
        client.publish(data_topic, printStr);
        //Debugln("Sent:");
        Debugln(printStr);
      } else {
        Debugln("Message too long to publish.");
      }
      memset(printStr, 0, sizeof(printStr));
      memset(storeStr, 0, sizeof(storeStr));
      memset(printStr, 0, sizeof(printStr));

    // Storing to microSD Card
    strcat(storeStr, "\n");
    appendFile(SD, sd_fileName.c_str(), storeStr);
    
    
    /*
    lc1vals = String(lc1Val);
    lc2vals = String(lc2Val);
    lc3vals = String(lc3Val);
    tc1vals = String(tc1Val);
    tc2vals = String(tc2Val);
  
    fillAcc = String(digitalRead(fill));
    dumpAcc = String(digitalRead(dump));
    ventAcc = String(digitalRead(vent));
    qdAcc = String(digitalRead(qd));
    mpvAcc = String(digitalRead(mpv));
  
    Debug("P1:");
    Debug(pt1vals);
    Debug(" P2:");
    Debug(pt2vals);
    Debug(" P3:");
    Debug(pt3vals);
    Debug(" P4:");
    Debug(pt4vals);
    Debug(" P5:");
    Debug(pt5vals);
    Debug(" LC1:");
    Debug(lc1vals);
    Debug(" AC1:");
    Debug(fillAcc);
    Debug(" AC2:");
    Debug(dumpAcc);
    Debug(" AC3:");
    Debug(ventAcc);
    Debug(" AC4:");
    Debug(qdAcc);
    Debug(" AC5:");
    Debugln(mpvAcc);
    
    rs485Debug("P1:");
    rs485Debug(pt1vals);
    rs485Debug(" P2:");
    rs485Debug(pt2vals);
    rs485Debug(" P3:");
    rs485Debug(pt3vals);
    rs485Debug(" P4:");
    rs485Debug(pt4vals);
    rs485Debug(" P5:");
    rs485Debug(pt5vals);
    rs485Debug(" LC1:");
    rs485Debug(lc1vals);
    rs485Debug(" AC1:");
    rs485Debug(fillAcc);
    rs485Debug(" AC2:");
    rs485Debug(dumpAcc);
    rs485Debug(" AC3:");
    rs485Debug(ventAcc);
    rs485Debug(" AC4:");
    rs485Debug(qdAcc);
    rs485Debug(" AC5:");
    rs485Debugln(mpvAcc); */


    last_time = millis();
  }
}

// MICROSD CARD READER FUNCTION DEFINITIONS

void readFile(fs::FS &fs, const char * path){
  Debugf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Debugln("Failed to open file for reading");
    return;
  }

  Debug("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Debugf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Debugln("Failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Debugln("File written");
  } else {
    Debugln("Write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Debugf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Debugln("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Debugln("Message appended");
  } else {
    Debugln("Append failed");
  }
  file.close();
}








////////// LOADCELL CALIBRATION CODE //////////

/*
#include "HX711.h"

#define hx1_data_pin  45
#define hx_sck_pin  37

HX711 lc1;

float calibration_factor = 18980; // Torque load cell
//float calibration_factor = -2585;// Tension load cell

void setup() {
  Serial.begin(9600);
  Debugln("HX711 calibration sketch");
  Debugln("Remove all weight from scale");
  Debugln("After readings begin, place known weight on scale");
  Debugln("Press + or a to increase calibration factor");
  Debugln("Press - or z to decrease calibration factor");

  lc1.begin(hx1_data_pin, hx_sck_pin);
  lc1.set_scale();
  lc1.tare(); //Reset the scale to 0

  long zero_factor = lc1.read_average(); //Get a baseline reading
  Debug("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Debugln(zero_factor);
}

void loop() {

  lc1.set_scale(calibration_factor); //Adjust to this calibration factor

  Debug("Reading: ");
  Debug(lc1.get_units());
  Debug(" lbs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Debug(" calibration_factor: ");
  Debug(calibration_factor);
  Debugln();

  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
  }
}*/
