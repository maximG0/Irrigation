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
void Channel::sortReadings(){//ascending
    for(int j=0;j<NUM_OF_READINGS-1;j++){
        for(int i=j+1;i<NUM_OF_READINGS;i++){
            if(lastReadings[j]>lastReadings[i]){
                int temp=lastReadings[i];
                lastReadings[i]=lastReadings[j];
                lastReadings[j]=temp;
            }
        }
    }
}
int Channel::calcAverageReading(){
    sortReadings();
    int usedReadings=0;
    int avgReading=0;
    for(int i=0;i<NUM_OF_READINGS;i++){
        if(i > NUM_OF_READINGS*0.2 && i < NUM_OF_READINGS*0.8){//filtering out top and bottom outliers
            avgReading+=lastReadings[i];
            usedReadings++;
        }
    }
    avgReading/=usedReadings;
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
void Channel::manual_irrigate(){
    state=2;
    valve.open();
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
        //readingChangeCallback();
        return;
    }
    turnOff();
    //readingChangeCallback();
}