#ifndef _CHANNEL_
#define _CHANNEL_

#include "MoistureSensor/MoistureSensor.h"
#include "Valve/Valve.h"
#include "Common.h"

#define NUM_OF_READINGS 20

class Channel{
public:
    Channel(int sensorPin,int valvePin);
    void irrigate();
    void manual_irrigate();
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
    int state; // 0 = not in time range or in range but wasn't turned-on yet; 1 = off-sensing; 2 = on-irrigating
    int lastReadings[NUM_OF_READINGS];//last N sensor readings
    int lastIndex;//last index read
    void sortReadings();
    int calcAverageReading();
    void resetReadings();
    int lastAverageReading;
    std::function<void()> readingChangeCallback;
    int threshold;
    int target;
};

#endif