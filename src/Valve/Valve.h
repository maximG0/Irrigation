#ifndef _VALVE_
#define _VALVE_

#include "Common.h"

class Valve{
public:
    Valve(int valvePin);
    void open();
    void close();
    bool is_open();
private:
    int pin;
    bool isOpen;
};

#endif