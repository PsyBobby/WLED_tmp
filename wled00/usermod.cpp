#include "wled.h"
/*
 * This v1 usermod file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in const.h)
 * If you just need 8 bytes, use 2551-2559 (you do not need to increase EEPSIZE)
 *
 * Consider the v2 usermod API if you need a more advanced feature set!
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

#include "Adafruit_SHT4x.h"

#include <Wire.h>

#include "SparkFun_SCD4x_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD4x


#include "RTClib.h"

#include "Adafruit_FRAM_I2C.h"

#define EEPROM_ADDR 0x50  // the default address!

Adafruit_FRAM_I2C i2ceeprom;

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

Adafruit_SHT4x sht4 = Adafruit_SHT4x();

SCD4x mySensor(SCD4x_SENSOR_SCD41); // Tell the library we have a SCD41 connected

#define RXD2 16
#define TXD2 17

float shtTemp = 0;
float shtHum = 0;
uint32_t shtDura = 0;

float scdTemp = 0; //19.2
float scdHum = 0; //37.9
int scdCO2 = 0; //1634

int scdTemp2 = 0;
int scdCO22 = 0;



//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
	Serial.begin(115200);
    while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //Serial2.begin(9600);
    while (!Serial2)
    delay(10);

  // RTC
    if (! rtc.begin()) {
     Serial.println("Couldn't find RTC");
     Serial.flush();
     while (1) delay(10);
   }

    if (rtc.lostPower()) {
     Serial.println("RTC lost power, let's set the time!");
      // When time needs to be set on a new device, or after a power loss, the
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }

    // When time needs to be re-set on a previously configured device, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  
  // Fram
    uint16_t num;
  
    Serial.begin(115200);
  
    if (i2ceeprom.begin(0x50)) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
      Serial.println("Found I2C EEPROM");
    } else {
      Serial.println("I2C EEPROM not identified ... check your connections?\r\n");
      while (1) delay(10);
    }

    String s = "Hello world!";
    num = i2ceeprom.writeObject(0x00, s);
    Serial.print("Wrote a string with ");
    Serial.print(num);
    Serial.println(" bytes");

    String s2;
    i2ceeprom.readObject(0x00, s2);
    Serial.print("Read back string value: ");
    Serial.print(s2);

 

  Serial.println("Adafruit SHT4x test");
  if (! sht4.begin()) {
    Serial.println("Couldn't find SHT4x");
    while (1) delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);

  // You can have 3 different precisions, higher precision takes longer
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  switch (sht4.getPrecision()) {
     case SHT4X_HIGH_PRECISION: 
       Serial.println("High precision");
       break;
     case SHT4X_MED_PRECISION: 
       Serial.println("Med precision");
       break;
     case SHT4X_LOW_PRECISION: 
       Serial.println("Low precision");
       break;
  }

  // You can have 6 different heater settings
  // higher heat and longer times uses more power
  // and reads will take longer too!
  sht4.setHeater(SHT4X_NO_HEATER);
  switch (sht4.getHeater()) {
     case SHT4X_NO_HEATER: 
       Serial.println("No heater");
       break;
     case SHT4X_HIGH_HEATER_1S: 
       Serial.println("High heat for 1 second");
       break;
     case SHT4X_HIGH_HEATER_100MS: 
       Serial.println("High heat for 0.1 second");
       break;
     case SHT4X_MED_HEATER_1S: 
       Serial.println("Medium heat for 1 second");
       break;
     case SHT4X_MED_HEATER_100MS: 
       Serial.println("Medium heat for 0.1 second");
       break;
     case SHT4X_LOW_HEATER_1S: 
       Serial.println("Low heat for 1 second");
       break;
     case SHT4X_LOW_HEATER_100MS: 
       Serial.println("Low heat for 0.1 second");
       break;
  }

   Serial.println(F("SCD41 Example"));
  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to get helpful debug messages on Serial

  if (mySensor.begin(false, true, false) == false) // Do not start periodic measurements
  //measBegin_________/     |     |
  //autoCalibrate__________/      |
  //skipStopPeriodicMeasurements_/
  {
    Serial.println(F("Sensor not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  //Let's call measureSingleShot to start the first conversion
  bool success = mySensor.measureSingleShot();
  if (success == false)
  {
    Serial.println(F("measureSingleShot failed. Are you sure you have a SCD41 connected? Freezing..."));
    while (1)
      ;    
	  }

}



//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
	// get Sensor Data
    sensors_event_t humidity, temp;
  
    uint32_t timestamp = millis();
    sht4.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    timestamp = millis() - timestamp;

    shtTemp = temp.temperature;
    shtHum = humidity.relative_humidity;

  // print Sensordata
    Serial.print("Temperature: "); Serial.print(shtTemp); Serial.println(" degrees C");
    Serial.print("Humidity: "); Serial.print(shtHum); Serial.println("% rH");

    Serial.print("Read duration (ms): ");
    Serial.println(timestamp);

  // Send Data to Nextion
    Serial2.print("t3.txt=\"" + String(shtTemp) + " C°" + "\"");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);

    Serial2.print("t4.txt=\"" + String(shtHum) + " % rH" + "\"");
    Serial2.write(0xff);
    Serial2.write(0xff);
    Serial2.write(0xff);

  // Read Nextion Slider Data
    if(Serial2.available()){
      char character = ' ';
      String data_from_display = "";
      delay(30);
      while(Serial2.available()){
        character = char(Serial2.read());
        data_from_display += character;
      }
      Serial.println(data_from_display);
      if(data_from_display.substring(0,3) == "red"){
        Serial.println(data_from_display.substring(3,9));
        int red = data_from_display.substring(3,9).toInt();
      }else if(data_from_display.substring(0,3) == "gre"){
        Serial.println(data_from_display.substring(3,9));
        int gre = data_from_display.substring(3,9).toInt();
      }else if(data_from_display.substring(0,3) == "blu"){
        Serial.println(data_from_display.substring(3,9));
        int blu = data_from_display.substring(3,9).toInt();
      }else if(data_from_display.substring(0,3) == "sat"){
        Serial.println(data_from_display.substring(3,9));
        int sat = data_from_display.substring(3,9).toInt();
        Serial.println(String(data_from_display.substring(3,9).toInt()));
      }else {
        Serial.println("faild to parse");
      }
    }

  // RTC
    DateTime now = rtc.now();

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");

    // calculate a date which is 7 days, 12 hours, 30 minutes, 6 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));

    Serial.print(" now + 7d + 12h + 30m + 6s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();

    Serial.print("Temperature: ");
    Serial.print(rtc.getTemperature());
    Serial.println(" C");

    Serial.println();


  while (mySensor.readMeasurement() == true) // readMeasurement will return true when fresh data is available
  {
  //  Serial.print(F("."));
  //  delay(500);
  //}

  

  Serial.println();

  Serial.print(F("CO2(ppm):"));
  Serial.print(mySensor.getCO2());

  Serial.print(F("\tTemperature(C):"));
  Serial.print(mySensor.getTemperature(), 1);

  Serial.print(F("\tHumidity(%RH):"));
  Serial.print(mySensor.getHumidity(), 1);

  Serial.println();
  
  scdTemp = mySensor.getTemperature();
  scdHum = mySensor.getHumidity();
  scdCO2 = mySensor.getCO2();

  scdTemp2 = (scdTemp * 4);
  scdCO22 = (scdCO2 / 5);
  //timestamp2 = millis() - timestamp2;
  //shtDura = timestamp2;

 

  Serial1.print("t2.txt=\"" + String(scdTemp) + " C°" + "\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);

  Serial1.print("t3.txt=\"" + String(scdHum) + " %rH" + "\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
  
  Serial1.print("t4.txt=\"" + String(scdCO2) + " ppm CO2" + "\"");
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);


  Serial1.print("z2.val=" + String(scdTemp2));
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);

  //debug print

  Serial.print("z2.val=" + String(scdTemp2));
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.println();


  Serial.print("t2.txt=\"" + String(scdTemp) + " C°" + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.println();

  Serial.print("t3.txt=\"" + String(scdHum) + " %rH" + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.println();
  
  Serial.print("t4.txt=\"" + String(scdCO2) + " ppm CO2" + "\"");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.println();

  Serial1.print("z4.val=" + String(scdCO22));
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);

  mySensor.measureSingleShot(); // Request fresh data (should take 5 seconds)

  }

}
