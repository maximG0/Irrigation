#include <credentials.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define DRY 3000
#define WET 900
#define NEED_WATER 2000
#define VALVE_1 23
#define SENSOR1_PIN 36
BlynkTimer timer;

void startWatering(){
  Blynk.virtualWrite(V2,HIGH);
  digitalWrite(VALVE_1,HIGH);
}
void stopWatering(){
  Blynk.virtualWrite(V2,LOW);
  digitalWrite(VALVE_1,LOW);
}

void updateSensorVal(){
  int sens_read_1 = analogRead(SENSOR1_PIN);
  Blynk.virtualWrite(V0,map(sens_read_1,WET,DRY,100,0));

  if(sens_read_1 > NEED_WATER){
    startWatering();
  }
  else{
    stopWatering();
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(VALVE_1,OUTPUT);
  pinMode(SENSOR1_PIN,INPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);
  timer.setInterval(1000L,updateSensorVal);
}

void loop() {
  Blynk.run();
  timer.run();
}
