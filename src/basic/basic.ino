#include <rn2xx3.h> 
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "keys.h"
SoftwareSerial myLoraSerial(7, 8); // RX, TX
#define RST  2
static const int buzzerPin = 5;
#define RXPin 4
#define TXPin 3
#define GPSPower 10
static const uint32_t GPSBaud = 9600;
// The serial connection to the GPS device
SoftwareSerial gpsSerial(RXPin, TXPin);

rn2xx3 myLora(myLoraSerial);

// The TinyGPS++ object
TinyGPSPlus gps;

unsigned long lastExecutionMillis = 0;
unsigned long lastSendMillis = 0;
unsigned long lastBeepMillis = 0;
#define accelX A0 // X pin connected to Analog 0
#define accelY A1 // Y pin connected to Analog 1
#define accelZ A2 // Z pin connected to Analog 2

#define tolerance 20 // Sensitivity of the Alarm

//Accelerometer limits
int xMin; //Minimum x Value
int xMax; //Maximum x Value
int xVal; //Current x Value

int yMin; //Minimum y Value
int yMax; //Maximum y Value
int yVal; //Current y Value

int zMin; //Minimum z Value
int zMax; //Maximum z Value
int zVal; //Current z Value


boolean gpsPowered = false;
// Setup routine runs once when you press reset
void setup() {
  pinMode(13, OUTPUT);
  led_on();
  pinMode(buzzerPin, OUTPUT); //Set buzzerPin as output

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  myLoraSerial.begin(9600);
  Serial.println("Startup");

  // Reset rn2xx3
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);
  digitalWrite(RST, LOW);
  delay(500);
  digitalWrite(RST, HIGH);

  // Initialise the rn2xx3 module
  myLora.autobaud();

  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(myLora.hweui());
  Serial.print("Microchip RN2xx3 version number: ");
  Serial.println(myLora.sysver());

  if(!myLora.initABP(devAddr, appSKey, nwkSKey)){
    Serial.println("Could not connect to LoRaWAN network");
  }
  
  delay(500);
  Serial.println("Calibrating Accelerometer...");
  calibrateAccel();
  Serial.println("Calibrated Accelerometer");
  
  led_off();
  delay(2000);
}
// Function used to calibrate the Accelerometer
void calibrateAccel(){

 //initialise x,y,z variables
 xVal = analogRead(accelX);
 xMin = xVal;
 xMax = xVal;
 
 yVal = analogRead(accelY);
 yMin = yVal;
 yMax = yVal;
 
 zVal = analogRead(accelZ);
 zMin = zVal;
 zMax = zVal;
 
 
 //calibrate the Accelerometer (should take about 0.5 seconds)
 for (int i=0; i<50; i++){
 // Calibrate X Values
 xVal = analogRead(accelX);
 if(xVal>xMax){
 xMax=xVal;
 }else if (xVal < xMin){
 xMin=xVal;
 }

 // Calibrate Y Values
 yVal = analogRead(accelY);
 if(yVal>yMax){
 yMax=yVal;
 }else if (yVal < yMin){
 yMin=yVal;
 }

 // Calibrate Z Values
 zVal = analogRead(accelZ);
 if(zVal>zMax){
 zMax=zVal;
 }else if (zVal < zMin){
 zMin=zVal;
 }

 //Delay 10msec between readings
 delay(10);
 }
 printValues(); //Only useful when connected to computer- using serial monitor.
}

boolean checkMotion(){
 boolean tempB=false;
 xVal = analogRead(accelX);
 yVal = analogRead(accelY);
 zVal = analogRead(accelZ);
 
 if(xVal >(xMax+tolerance)||xVal < (xMin-tolerance)){
 tempB=true;
 Serial.print("X Failed = ");
 Serial.println(xVal);
 }
 
 if(yVal >(yMax+tolerance)||yVal < (yMin-tolerance)){
 tempB=true;
 Serial.print("Y Failed = ");
 Serial.println(yVal);
 }
 
 if(zVal >(zMax+tolerance)||zVal < (zMin-tolerance)){
 tempB=true;
 Serial.print("Z Failed = ");
 Serial.println(zVal);
 }
 
 return tempB;
}
 
// the loop routine runs over and over again forever:
void loop() {
  int currentMillis = millis(); 
  if(!gpsPowered && checkMotion()){
    powerGps();
  }

  if(!gpsPowered){
    return;
  }
  //if(currentMillis - lastExecutionMillis < 5000){
  //  return;
  //}
  lastExecutionMillis = currentMillis; 
  int available = gpsSerial.available();
  if(available == 0){
    return;
  }
  led_on();
    
  Serial.println("available bytes from gps: " + String(available, DEC));
  while (available > 0){
    if (gps.encode(gpsSerial.read())){
      displayInfo();
      if (gps.location.isValid() && (lastSendMillis == 0 || currentMillis - lastSendMillis > 30000)){
        lastSendMillis = currentMillis;
        transmit_coords(gps.location.lat(), gps.location.lng());
      }
      break;
    }
    available = gpsSerial.available();
  }
  if(receivedBikeKillSignal()){
    Serial.println("Deactivating Bike...");
    if(lastBeepMillis == 0 || currentMillis - lastBeepMillis > 5000) {
      lastBeepMillis = currentMillis;
      beep();
    }
  }
  led_off();
}
void powerGps() {
  digitalWrite(GPSPower,1);
  delay(200);
  Serial.println("Initializing GPS...");
  gpsSerial.begin(GPSBaud);
  Serial.println("Initialized GPS");
  gpsPowered = true;
}
boolean receivedBikeKillSignal() {
  String response = myLora.getRx();
  if(response && response.length() > 0) {
    Serial.println("Response: " + response);
    return response.toInt() == 31;
  }
  return false;
}

void beep() { //creating function
    for(int i=0; i<5; i++) {
      tone(buzzerPin, 1410, 800);
      delay(800);
      tone(buzzerPin, 1000, 800);
      delay(800);
    }
}

void transmit_coords(double float_latitude, double float_longitude){
  Serial.println("Transimitting coords...");
  uint8_t coords[7]; // 6*8 bits = 48
  int32_t lat = float_latitude * 10000;
  int32_t lon = float_longitude * 10000;

  // Pad 2 int32_t to 6 8uint_t, skipping the last byte (x >> 24)
  coords[0] = lat;
  coords[1] = lat >> 8;
  coords[2] = lat >> 16;
  
  coords[3] = lon;
  coords[4] = lon >> 8;
  coords[5] = lon >> 16;
  coords[6] = receivedBikeKillSignal();
  myLoraSerial.listen();
  TX_RETURN_TYPE result = myLora.txBytes(coords, sizeof(coords));
  if(result == TX_FAIL)
  {
    Serial.println("Failed to send data!");
  }
  gpsSerial.listen();
}


void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
}

void printValues(){
 Serial.print("xMin=");
 Serial.print(xMin);
 Serial.print(", xMax=");
 Serial.print(xMax);
 Serial.println();
 
 Serial.print("yMin=");
 Serial.print(yMin);
 Serial.print(", yMax=");
 Serial.print(yMax);
 Serial.println();
 
 Serial.print("zMin=");
 Serial.print(zMin);
 Serial.print(", zMax=");
 Serial.print(zMax);
 Serial.println();
 
 Serial.println("------------------------");
}


void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

 Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

 Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

 Serial.println();
}
