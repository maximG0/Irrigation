#include "MoistureSensor.h"


MoistureSensor::MoistureSensor(int sensorPin):pin(sensorPin){
    pinMode(pin,INPUT);
}
int MoistureSensor::get_value(){
    return analogRead(pin);
}