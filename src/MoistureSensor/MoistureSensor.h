#ifndef _SENSOR_
#define _SENSOR_

#include "Common.h"

class MoistureSensor
{
public:
    MoistureSensor(int sensorPin);
    int get_value();
private:
    int pin;
};

#endif