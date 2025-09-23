#ifndef _COMMON_
#define _COMMON_

#include <Arduino.h>

#define DRY 2000
#define WET 1000
#define NEED_WATER 2000
#define TARGET_MOISTURE 1250
#define DEFAULT_START_TIME 25200 //7AM
#define DEFAULT_STOP_TIME 27000 //7:30
#define MIN_VALID_TIMESTAMP 1690000000 //around 2023

//esp physical sensors
#define LARGE_SENSOR_PIN 36

//esp physical valves
#define LARGE_VALVE_PIN 25

//blynk virtual pins
#define BLYNK_LARGE_SENSOR 0
#define BLYNK_LARGE_VALVE 2
#define BLYNK_THRESHOLD 5
#endif