#include "credentials.h"
#include "Common.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <iostream>
#include <ctime>
#include "MoistureSensor/MoistureSensor.h"
#include "Valve/Valve.h"
#include "Channel/Channel.h"

BlynkTimer timer;
Channel largePlants(LARGE_SENSOR_PIN,LARGE_VALVE_PIN);
int moistureThreshold = NEED_WATER;
int targetMoisture = TARGET_MOISTURE;
int localTime;//in seconds
int startTime=DEFAULT_START_TIME;
int stopTime=DEFAULT_STOP_TIME;

void updateBlynk(){
  Blynk.virtualWrite(V0,map(largePlants.getSensorReading(),WET,DRY,100,0));
  Blynk.virtualWrite(V2,largePlants.get_state());
  Blynk.virtualWrite(V4,largePlants.getSensorReading());
}
void checkTime(){
  if(startTime<=localTime && localTime<=stopTime){
    largePlants.inRange=true;
  }
  else{
    largePlants.inRange=false;
  }
}

void irrigate(){
  Blynk.sendInternal("rtc","sync");//request local time
  checkTime();
  largePlants.irrigate();
}

BLYNK_WRITE(V5){
  //threshold was changed in the app settings
  moistureThreshold = map(param.asInt(),0,100,DRY,WET);
  largePlants.updateThreshold(moistureThreshold);
}
BLYNK_WRITE(V8){
  //target moisture was changed
  targetMoisture = map(param.asInt(),0,100,DRY,WET);
  largePlants.updateTarget(targetMoisture);
}
BLYNK_WRITE(InternalPinRTC){
  //local time update
  //getting it in unix format and converting to seconds
  time_t unixTime = param.asLong();
  struct tm* timeInfo = gmtime(&unixTime);
  localTime = timeInfo->tm_hour*3600 + timeInfo->tm_min*60 + timeInfo->tm_sec;
}
BLYNK_WRITE(V6){
  //user input time update
  startTime=param[0].asInt();
  stopTime=param[1].asInt();
}
BLYNK_CONNECTED(){
  Blynk.syncVirtual(V5,V6,V8);
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);
  timer.setInterval(500L,irrigate);
  largePlants.setCallback(updateBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}
