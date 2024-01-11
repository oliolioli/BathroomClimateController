#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <Arduino.h>

void setupAP();
void loopAP();

void setHumidity(unsigned short& hum);
void setTemperature(float& temp);
void setDew(float& temp, unsigned short& hum);
void setTempHistory(float tempHistory[], int size);
void setHumidityHistory(unsigned short humHistory[], int size);
int ledDisplay(float& temp, unsigned short& hum);
#endif
