#include "Channel.h"

Channel::Channel(int sensorPin,int valvePin):moistureSensor(sensorPin),
        valve(valvePin),state(0),lastReadings{0},lastIndex(0),threshold(NEED_WATER),target(TARGET_MOISTURE),inRange(0) {}

void Channel::resetReadings(){
    lastIndex=0;
    for(int i=0;i<NUM_OF_READINGS;i++)lastReadings[i]=0;
}

int Channel::getSensorReading(){
    return lastAverageReading;
}

int Channel::calcAverageReading(){
    int avgReading=0;
    for(int i=0;i<NUM_OF_READINGS;i++){
        avgReading+=lastReadings[i];
    }
    avgReading/=NUM_OF_READINGS;
    lastAverageReading=avgReading;
    resetReadings();
    return avgReading;
}

void Channel::setCallback(std::function<void()> callback){
    readingChangeCallback=callback;
}

void Channel::updateThreshold(int value){threshold=value;}

void Channel::updateTarget(int value){target=value;}

int Channel::get_state(){return state;}

void Channel::turnOff(){
    state=0;
    valve.close();
}
void Channel::irrigate(){
    if(lastIndex<NUM_OF_READINGS){//gathering readings
        lastReadings[lastIndex]=moistureSensor.get_value();
        lastIndex++;
        return;
    }
    int average=calcAverageReading();
    if(inRange){
        switch (state){
            case 0:
                //just entered time range
                if(average>threshold){
                    state=2;
                    valve.open();
                }
                break;
            case 1:
                if(average>target){
                    state=2;
                    valve.open();
                }
                break;
            case 2:
                if(average<=target){
                    state=1;
                    valve.close();
                }
                break;
        }
        readingChangeCallback();
        return;
    }
    turnOff();
    readingChangeCallback();
}