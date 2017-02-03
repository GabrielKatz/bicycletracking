#include <rn2xx3.h> 
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "keys.h"
SoftwareSerial myLoraSerial(7, 8); // RX, TX
#define RST  2
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
// The serial connection to the GPS device
SoftwareSerial gpsSerial(RXPin, TXPin);

rn2xx3 myLora(myLoraSerial);

// The TinyGPS++ object
TinyGPSPlus gps;

int lastExecutionMillis = 0;
int lastSendMillis = 0;
// Setup routine runs once when you press reset
void setup() {
  pinMode(13, OUTPUT);
  led_on();

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
  Serial.println("Initializing GPS...");
  gpsSerial.begin(GPSBaud);
  Serial.println("Initialized GPS");
  
  led_off();
}

// the loop routine runs over and over again forever:
void loop() {
  int currentMillis = millis(); 

  //if(currentMillis - lastExecutionMillis < 5000){
  //  return;
  //}
  lastExecutionMillis = currentMillis; 
  led_on();
  int available = gpsSerial.available();
  if(available == 0)
    return;
  Serial.println("available bytes from gps: " + String(available, DEC));
  while (available > 0){
    if (gps.encode(gpsSerial.read())){
      displayInfo();
      if (gps.location.isValid() && currentMillis - lastSendMillis > 30000){
        transmit_coords(gps.location.lat(), gps.location.lng());
      }
      break;
    }
    available = gpsSerial.available();
  }
  if(receivedBikeKillSignal()){
    Serial.println("Deactivating Bike...");
  }
  led_off();
}

boolean receivedBikeKillSignal() {
  String response = myLora.getRx();
  if(response && response.length() > 0) {
    Serial.println("Response: " + response);
    return response.toInt() == 31;
  }
  return false;
}

void transmit_coords(double float_latitude, double float_longitude){
  Serial.println("Transimitting coords...");
  uint8_t coords[6]; // 6*8 bits = 48
  int32_t lat = float_latitude * 10000;
  int32_t lon = float_longitude * 10000;

  // Pad 2 int32_t to 6 8uint_t, skipping the last byte (x >> 24)
  coords[0] = lat;
  coords[1] = lat >> 8;
  coords[2] = lat >> 16;
  
  coords[3] = lon;
  coords[4] = lon >> 8;
  coords[5] = lon >> 16;
  myLora.txBytes(coords, sizeof(coords));
}
void led_on()
{
  digitalWrite(13, 1);
}

void led_off()
{
  digitalWrite(13, 0);
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
