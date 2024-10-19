#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);  
uint8_t RXPin = 17;   // Connect this to TX of GPS
uint8_t TXPin = 5;   // Connect this to RX of GPS (if needed)

void initGPS() {
  gpsSerial.begin(9600, SERIAL_8N1, RXPin, TXPin);  // Start GPS communication
}

void getInfo() {
  // Read data from GPS module
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());

    // If new location data is available, print it
    if (gps.location.isUpdated()) {
      Serial.print("Latitude: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("Altitude: ");
      Serial.println(gps.altitude.meters());
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());
      Serial.print("time: ");
      Serial.println(gps.time.value());
      Serial.print("HDOP: ");
      Serial.println(gps.hdop.value());
      Serial.print("speed: ");
      Serial.println(gps.speed.kmph());
    }
  }
}

void getSat(){

}
