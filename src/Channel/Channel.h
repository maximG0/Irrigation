#ifndef _CHANNEL_
#define _CHANNEL_

#include "MoistureSensor/MoistureSensor.h"
#include "Valve/Valve.h"
#include "Common.h"

#define NUM_OF_READINGS 10

class Channel{
public:
    Channel(int sensorPin,int valvePin);
    void irrigate();
    int get_state();
    int getSensorReading();
    void setCallback(std::function<void()>);
    void updateThreshold(int value);
    void updateTarget(int value);
    void turnOff();
    bool inRange;//in time range set by user
private:
    MoistureSensor moistureSensor;
    Valve valve;
    int state; // 0 = passive; 1 = active_sensing; 2 = active_irrigating
    int lastReadings[NUM_OF_READINGS];//used to sample a few times to get a reliable average
    int lastIndex;//last index read
    int calcAverageReading();
    void resetReadings();
    int lastAverageReading;
    std::function<void()> readingChangeCallback;
    int threshold;
    int target;
};

#endif