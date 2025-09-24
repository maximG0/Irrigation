#include "credentials.h"

#define ENABLE_DATABASE
#define ENABLE_USER_AUTH

#include "Common.h"
#include <FirebaseClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <iostream>
#include <ctime>
#include "MoistureSensor/MoistureSensor.h"
#include "Valve/Valve.h"
#include "Channel/Channel.h"

Channel largePlants(LARGE_SENSOR_PIN,LARGE_VALVE_PIN);
int moistureThreshold = NEED_WATER;
int targetMoisture = TARGET_MOISTURE;
int localTime;//in seconds
int startTime=DEFAULT_START_TIME;
int stopTime=DEFAULT_STOP_TIME;
time_t now;
int valve_state=0;
int prev_valve_state=0;
int duration=0;
int interval_days=1;
time_t lastTime;
String raw_last_time="";
int schedHour=0;
int schedMin=0;
bool manual=0;
unsigned long manualStartMillis=0;
bool log_manual=0;
unsigned long manual_duration=0;


// Timer variables for sending data every 2 seconds
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000; // 2 seconds in milliseconds

// Firebase components
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASS);
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase rtdb;


void processData(AsyncResult &aResult);

void checkTime(){
  now = time(nullptr);
  struct tm* now_tm = localtime(&now);
  int currHour = now_tm->tm_hour;
  int currMin  = now_tm->tm_min;
  int time_since=int(difftime(now,lastTime));
  if(time_since>MIN_VALID_TIMESTAMP){
    return;
  }
  int days_since=time_since/86400;
  if(days_since<interval_days){
    largePlants.inRange=false;
    if(valve_state){
      rtdb.set<bool>(aClient, "/valve1", false, processData, "RTDB_Send_Bool");
    }
    return;
  }
  Serial.printf("sched:%d:%d, now:%d,%d, days passed:%d",schedHour,schedMin,currHour,currMin,days_since);
  if(currHour==schedHour && currMin>=schedMin && currMin<=schedMin+duration){
    largePlants.inRange=true;
    if(!valve_state){
      rtdb.set<bool>(aClient, "/valve1", true, processData, "RTDB_Send_Bool");
    }
  }
  else if(currHour==schedHour+1 && schedMin+duration>59 && schedMin+duration-60>=currMin){
    largePlants.inRange=true;
    if(!valve_state){
      rtdb.set<bool>(aClient, "/valve1", true, processData, "RTDB_Send_Bool");
    }
  }
  else{
    largePlants.inRange=false;
    if(valve_state){
      rtdb.set<bool>(aClient, "/valve1", false, processData, "RTDB_Send_Bool");
    }
  }
}
void logIrrigation(AsyncClient& aClient, int valve, int duration) {
  now = time(nullptr);
  struct tm* tm_info = localtime(&now);
  char datetimeStr[25];
  sprintf(datetimeStr, "%04d-%02d-%02dT%02d:%02d:%02d",
          tm_info->tm_year + 1900,
          tm_info->tm_mon + 1,
          tm_info->tm_mday,
          tm_info->tm_hour,
          tm_info->tm_min,
          tm_info->tm_sec);

  rtdb.set<String>(aClient, "/last_irrigation/datetime", datetimeStr, processData, "RTDB_Send_String");
  rtdb.set<int>(aClient, "/last_irrigation/duration", duration, processData, "RTDB_Send_Int");
}
void irrigate(){
  checkTime();
  largePlants.irrigate();
}
void manualOn(){
  largePlants.manual_irrigate();
  manualStartMillis=millis();
}
void manualOff(){
  largePlants.turnOff();
  manual_duration=(millis()-manualStartMillis)/(1000);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID,WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  configTime(3*3600, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < MIN_VALID_TIMESTAMP) { // wait until NTP sync
    delay(300);
    Serial.print(".");
    now = time(nullptr);
  }
  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setHandshakeTimeout(5);
  
  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(rtdb);
  rtdb.url(DATABASE_URL);
}

void loop() {
  app.loop();
  if(app.ready()){
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval){
      int moisture = largePlants.getSensorReading();
      lastSendTime = currentTime;
      // Read the state from Firebase RTDB
      rtdb.get(aClient, "/valve1", processData, false, "RTDB_GetValveState");
      rtdb.get(aClient, "/manual", processData, false, "RTDB_GetManual");
      rtdb.get(aClient, "/settings/duration_seconds", processData, false, "RTDB_GetDuration");
      rtdb.get(aClient, "/settings/interval_days", processData, false, "RTDB_GetInterval");
      rtdb.get(aClient, "/settings/time", processData, false, "RTDB_GetTime");
      rtdb.get(aClient, "/last_irrigation/datetime", processData, false, "RTDB_GetLastTime");
      rtdb.get(aClient, "/settings/threshold", processData, false, "RTDB_GetThreshold");
      rtdb.set<int>(aClient, "/sensors/sensor1", moisture, processData, "RTDB_Send_Int");
      if(!manual){
        irrigate();
        if(log_manual){
            logIrrigation(aClient,1,manual_duration);
            log_manual=0;
        }
        if(valve_state){
          if(!prev_valve_state){
            logIrrigation(aClient,1,duration);
            prev_valve_state=1;
          }
        }else{
          prev_valve_state=0;
        }
      }
    }
  }
}
time_t parseISO8601(const char* datetimeRaw) {
  String dt = String(datetimeRaw);
  dt.trim();
  if (dt.startsWith("\"") && dt.endsWith("\"")) {
    dt = dt.substring(1, dt.length() - 1); // remove quotes
  }

  struct tm tm_info = {0};
  int parsed = sscanf(dt.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d",
                      &tm_info.tm_year,
                      &tm_info.tm_mon,
                      &tm_info.tm_mday,
                      &tm_info.tm_hour,
                      &tm_info.tm_min,
                      &tm_info.tm_sec);

  if (parsed != 6) {
    Serial.printf("parseISO8601: Failed to parse (%s)\n", dt.c_str());
    return 0;
  }

  tm_info.tm_year -= 1900;
  tm_info.tm_mon  -= 1;
  return mktime(&tm_info);
}

void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available()){
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
    String payload = aResult.c_str();
    if (aResult.uid() == "RTDB_GetValveState"){
      if(payload=="true"){
        valve_state=1;
      }else{
        valve_state=0;
        prev_valve_state=0;
      }
    }
    if (aResult.uid() == "RTDB_GetDuration"){
      duration = payload.toInt();
    }
    if (aResult.uid() == "RTDB_GetManual"){
      if(payload=="true"){
        manual=1;
        manualOn();
      }else{
        if(manual){
          log_manual=1;
          manualOff();
          manual=0;
        }
      }
    }
    if (aResult.uid() == "RTDB_GetInterval"){
      interval_days = payload.toInt();
    }
    if (aResult.uid() == "RTDB_GetThreshold"){
      moistureThreshold = payload.toInt();
      largePlants.updateThreshold(moistureThreshold);
    }
    if (aResult.uid() == "RTDB_GetLastTime"){
      if(raw_last_time!=payload){
        raw_last_time=payload;
        lastTime = parseISO8601(payload.c_str());
      }
    }
    if (aResult.uid() == "RTDB_GetTime"){
      String clean = payload;
      clean.replace("\"", ""); // remove quotes
      sscanf(clean.c_str(), "%d:%d", &schedHour, &schedMin);
    }
  }
}