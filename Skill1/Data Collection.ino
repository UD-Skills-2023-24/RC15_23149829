// import all the relevant libraries first
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include "Air_Quality_Sensor.h"


// set the variables for the gps sensor
SoftwareSerial serial_connection(2, 3);
TinyGPSPlus gps;

// variable for the SD card
const int chipSelect = 4;
File dataFile;
String filename = "Data.csv";
String Data = ""; // This is the string that will hold all the data. Please consider that this string can not be longer than about 140 characters. if you want to have more than that, then create two or more strings.
String Header = "index \t lat \t long \t speed \t date \t time \t air_val \t dust \t gsr";

// just the index
int index = 0;

// air quality sensor, sets the pin
AirQualitySensor sensor(A0);

// variable for the dust sensor, sets the pin
int pinDust = 8; //this is the pin that recieves incoming data
unsigned long duration; // this is the duration of the measurement
unsigned long starttime; // this is the starttime
unsigned long sampletime_ms = 30000;// it takes measures for 30s
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0; // this is the start value of the current concentration
float maxcon = 0;
//variables for the GSR sensor
const int GSRPIN = A3;
int gsr_sensorValue = 0;
int gsr_average = 0;



void setup() {
  // general setup
  Serial.begin(9600);//This opens up communications to the Serial monitor in the Arduino IDE and sets the speed. This is basically the connection to the computer
  // this are the setup statements for the dust sensor
  pinMode(pinDust, INPUT);
  starttime = millis();// milliseconds since the Arduino board began running the current program. in this case it starts at 0
  // this is the communication to the GPS
  serial_connection.begin(9600);//This opens up communications to the GPS
  // setup for the air sensor
  Serial.println("Air Sensor Init 20sec.");
  delay(20000); // and let's wait for the sensor to be ready
  if (sensor.init()) { // this statement executes after 20s, it checks if anyting has come through
    Serial.println("Air Sensor ready.");
  } else {
    Serial.println("Air Sensor ERROR!");
  }
  // setup for the SD card
  SD.begin(chipSelect);
  SD.remove(filename); // this statement deletes the prevous file with the same name
  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile)
  { dataFile.println(Header);
    dataFile.close();
  }
  Serial.println(Header);//and these are the values

  Serial.println("Setup finished");
}


void loop()
{
  while (serial_connection.available()) {
    gps.encode(serial_connection.read());
  }

  if (gps.location.isUpdated()) //if this object has been updated, do the following
  { Data = "";
  Serial.println("latitude");
  Serial.println(String(gps.location.lat(), 6));
  Serial.println("longitude");
  Serial.println(String(gps.location.lng(), 6));
    //this is the index
    Data.concat(index);
    // this is the data from the gps
    Data.concat("\t");
    Data.concat(String(gps.location.lat(), 6));
    Data.concat("\t");
    Data.concat(String(gps.location.lng(), 6));
    Data.concat("\t");
    Data.concat(gps.speed.mph());
    Data.concat("\t");
    Data.concat(gps.date.value());
    Data.concat("\t");
    Data.concat(gps.time.value());
    // this is the data from the air sensor
    int quality = sensor.slope();
    Data.concat("\t");
    Data.concat(sensor.getValue());
    // this is the data from the dust sensor
    duration = pulseIn(pinDust, LOW);
    lowpulseoccupancy = lowpulseoccupancy + duration;
    if ((millis() - starttime) > sampletime_ms) //if the sampel time == 30s
    { ratio = lowpulseoccupancy / (sampletime_ms * 10.0); // Integer percentage 0=>100
      concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62; // using spec sheet curve
      lowpulseoccupancy = 0;
      starttime = millis();
    }
    Data.concat("\t");
    Data.concat(concentration);
    // this is the data from the GSR sensor
    long gsr_sum = 0;
    for (int i = 0; i < 10; i++)    //Average the 10 measurements to remove the glitch
    { gsr_sensorValue = analogRead(GSRPIN);
      gsr_sum += gsr_sensorValue;
      delay(5);
    }
    gsr_average = gsr_sum / 10;
    Data.concat("\t");
    Data.concat(gsr_average);

    dataFile = SD.open(filename, FILE_WRITE); // the last event of this loop is that the data is written to the SD card
    if (dataFile)
    {
      dataFile.println(Data);
      dataFile.close();
      Serial.print(Data);
      Serial.println("\t \t Data written");
    }
    index = index + 1;
  }
  // in case you don't want so many data per minute it's not a bad idea to delay the sketch for a little white
  // delay(1000);
}
