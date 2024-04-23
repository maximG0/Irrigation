#include "Valve.h"

Valve::Valve(int valvePin):pin(valvePin){
    pinMode(pin,OUTPUT);
    close();
}
void Valve::close(){
    isOpen=0;
    //Blynk.virtualWrite(V2,LOW); //need to genrelise this
    digitalWrite(pin,LOW);
}
void Valve::open(){
    isOpen=1;
    //Blynk.virtualWrite(V2,HIGH); //and this
    digitalWrite(pin,HIGH);
}
bool Valve::is_open(){
    return isOpen;
}