#define BLYNK_PRINT Serial
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

BlynkTimer timer;
WidgetRTC rtc;

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); 

const int RELAY_PIN = D9;
const int RELAY_PIN1 = D8;

int pinValue;
int pinModeValue1 = 0;

int Shour;
int Smin;
int Ss;
int Ehour;
int Emin;
int Es;

char ssid[] = "Telia-BB3AE3-Greitas";
char pass[] = "427E50A40A";
char auth[] = "LW2_wDrAhk9uFk8UdLuDwCIlMKr0X3of";

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Sensor:       ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:   ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:    ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:    ")); Serial.print(sensor.max_value); Serial.println(F(" lux"));
  Serial.print  (F("Min Value:    ")); Serial.print(sensor.min_value); Serial.println(F(" lux"));
  Serial.print  (F("Resolution:   ")); Serial.print(sensor.resolution, 4); Serial.println(F(" lux"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
  delay(500);
}

/*
    Configures the gain and integration time for the TSL2591
*/
/****************************************************************************************************************************************************************************************/
void configureSensor(void)
{
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  //tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */
  Serial.println(F("------------------------------------"));
  Serial.print  (F("Gain:         "));
  tsl2591Gain_t gain = tsl.getGain();
  switch (gain)
  {
    case TSL2591_GAIN_LOW:
      Serial.println(F("1x (Low)"));
      break;
    case TSL2591_GAIN_MED:
      Serial.println(F("25x (Medium)"));
      break;
    case TSL2591_GAIN_HIGH:
      Serial.println(F("428x (High)"));
      break;
    case TSL2591_GAIN_MAX:
      Serial.println(F("9876x (Max)"));
      break;
  }
  Serial.print  (F("Timing:       "));
  Serial.print((tsl.getTiming() + 1) * 100, DEC);
  Serial.println(F(" ms"));
  Serial.println(F("------------------------------------"));
  Serial.println(F(""));
}

/*****************************************************************************************************************************************************************************************/

void advancedRead(void)
{
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
 int average = 0;
  int val;
  for (int i = 0; i < 10; i++) {
    ir = lum >> 16;
    full = lum & 0xFFFF;
    val = tsl.calculateLux(full, ir);
    average = average + val;
    delay(100);
  }
  int  AmbientPPFD = average  / 556;
 
  Blynk.virtualWrite(V5, AmbientPPFD);  
  
     if (AmbientPPFD > 150){
           pinModeValue1 = 1;
digitalWrite(RELAY_PIN, HIGH);
digitalWrite(RELAY_PIN1, HIGH); 
  }
  else{
    pinModeValue1 = 0;
  }
}

/*************************************************************************************************************************************************************************************/

BLYNK_CONNECTED()
{
  rtc.begin();
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details
  if (pinModeValue1 == 0) {
    String currentTime = String(hour()) + ":" + minute() + ":" + second();
    String currentDate = String(day()) + " " + month() + " " + year();
    int h = hour();
    int m = minute();
    Serial.print("Current time: ");
    Serial.print(currentTime);
    Serial.print(" ");
    Serial.print(currentDate);
    Serial.println();

    //works only if shour is less than end hour and for all days (make two if blocks comparing start and end hours and wrap code below in it), if want to do different days, add function(top of programme) to see which day is it and then compare,
    if (h > Shour && h < Ehour) {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(RELAY_PIN1, LOW);
    }
    if (h < Shour && h < Ehour) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(RELAY_PIN1, HIGH);
      digitalWrite(RELAY_PIN1, HIGH);
    }
    if (h > Shour && h > Ehour) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(RELAY_PIN1, HIGH);
      digitalWrite(RELAY_PIN1, HIGH);
    }
    if (h == Ehour && h == Shour ) {
      if (m >= Emin || m < Smin) {
        digitalWrite(RELAY_PIN, HIGH);
        digitalWrite(RELAY_PIN1, HIGH);
      }
      if (m < Emin && m >= Smin) {
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(RELAY_PIN1, LOW);
      }
    }
    if (h == Ehour && h > Shour ) {
      if (m >= Emin) {
        digitalWrite(RELAY_PIN, HIGH);
        digitalWrite(RELAY_PIN1, HIGH);
      }
      if (m < Emin) {
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(RELAY_PIN1, LOW);
      }
    }
    if (h < Ehour && h == Shour ) {
      if ( m < Smin) {
        digitalWrite(RELAY_PIN, HIGH);
        digitalWrite(RELAY_PIN1, HIGH);
      }
      if ( m >= Smin) {
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(RELAY_PIN1, LOW);
      }
    }
  }
}


BLYNK_WRITE(V1) {

  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    Serial.println(String("Start: ") +
                   t.getStartHour() + ":" +
                   t.getStartMinute() + ":" +
                   t.getStartSecond());
    delay(500);
    Shour = t.getStartHour();
    Blynk.virtualWrite(V7, Shour);
    Smin = t.getStartMinute();
    Blynk.virtualWrite(V8, Smin);
    Ss =  t.getStartSecond();
    Blynk.virtualWrite(V9, Ss);
    delay(1000);
  }
  else if (t.isStartSunrise())
  {
    Serial.println("Start at sunrise");
  }
  else if (t.isStartSunset())
  {
    Serial.println("Start at sunset");
  }
  else
  {
    // Do nothing
  }

  // Process stop time

  if (t.hasStopTime())
  {
    Serial.println(String("Stop: ") +
                   t.getStopHour() + ":" +
                   t.getStopMinute() + ":" +
                   t.getStopSecond());
    delay(500);
    Ehour = t.getStopHour();
    Blynk.virtualWrite(V10, Ehour);
    Emin = t.getStopMinute();
    Blynk.virtualWrite(V11, Emin);
    Es = t.getStopSecond();
    Blynk.virtualWrite(V12, Es);
  }
  else if (t.isStopSunrise())
  {
    Serial.println("Stop at sunrise");
  }
  else if (t.isStopSunset())
  {
    Serial.println("Stop at sunset");
  }
  else
  {
    // Do nothing: no stop time was set
  }

  // Process timezone
  // Timezone is already added to start/stop time

  Serial.println(String("Time zone: ") + t.getTZ());

  // Get timezone offset (in seconds)
  Serial.println(String("Time zone offset: ") + t.getTZ_Offset());

  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (int i = 1; i <= 7; i++) {
    if (t.isWeekdaySelected(i)) {
      Serial.println(String("Day ") + i + " is selected");
    }
  }
  Serial.println();
}



BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); 
  if ( pinValue = 1) {
    if (int(pinModeValue1)  % 2 == 0) {
      //
    }
    else {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(RELAY_PIN1, LOW);
    }
  }
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt(); 
  if ( pinValue = 1) {
    if (int(pinModeValue1)  % 2 == 0) {
      //
    }
    else {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(RELAY_PIN1, HIGH);
    }
  }
}

BLYNK_WRITE(V0) {
  switch (param.asInt())
  {
    case 1: { // Item 1
    pinModeValue1 = 0;
        break;
      }
    case 2: { // Item 2
        pinModeValue1 = 1;
        break;
      }   
    }
}

BLYNK_WRITE(V7) 
{
  int pinValue0 = param.asInt();
  Shour = pinValue0;
  Serial.println("V7 pin value: ");
  Serial.println(pinValue0);
}
BLYNK_WRITE(V8) 
{
  int pinValue1 = param.asInt();
  Smin = pinValue1;
  Serial.println("V8 pin value: ");
  Serial.println(pinValue1);
}
BLYNK_WRITE(V9) 
{
  int pinValue2 = param.asInt();
  Ss = pinValue2;
  Serial.println("V9 pin value: ");
  Serial.println(pinValue2);
}
BLYNK_WRITE(V10) 
{
  int pinValue3 = param.asInt();
  Ehour = pinValue3;
  Serial.println("V10 pin value: ");
  Serial.println(pinValue3);
}
BLYNK_WRITE(V11) 
{
  int pinValue3 = param.asInt();
  Emin = pinValue3;
  Serial.println("V11 pin value: ");
  Serial.println(pinValue3);
}
BLYNK_WRITE(V12) 
{
  int pinValue4 = param.asInt();
  Es = pinValue4;
  Serial.println("V12 pin value: ");
  Serial.println(pinValue4);
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(RELAY_PIN1, OUTPUT);
  digitalWrite(RELAY_PIN1, HIGH);

  Serial.println(F("Starting Adafruit TSL2591 Test!"));

  if (tsl.begin())
  {
    Serial.println(F("Found a TSL2591 sensor"));
  }
  else
  {
    Serial.println(F("No sensor found ... check your wiring?"));
    while (1);
  }
  /* Display some basic information on this sensor */
  displaySensorDetails();
  /* Configure the sensor */
  configureSensor();

  Serial.println("Start sync");
  delay(200);
  Blynk.syncVirtual(V0);
  delay(200);
  Blynk.syncVirtual(V10);
  delay(200);
  Blynk.syncVirtual(V11);
  delay(200);
  Blynk.syncVirtual(V12);
  delay(200);
  Blynk.syncVirtual(V7);
  delay(200);
  Blynk.syncVirtual(V8);
  delay(200);
  Blynk.syncVirtual(V9);
  delay(200);
 

  setSyncInterval(20 * 60); // Sync interval in seconds (10 minutes)
  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
  timer.setInterval(30000L, advancedRead);

}

void loop()
{
  Blynk.run();
  timer.run();
}
