#ifndef MANAGER_H
#define MANAGER_H

#include <Arduino.h>
#include <MsTimer2.h>
#include <SPI.h>

// Oversamples data by a factor of 160
float analogReadAve(void);
float testDAC(int index);
float analogReadDitherAve(int index);
void recurseStats1(float smpl, float &mean, float &stdev);
void recurseStats2(float smpl, float &mean, float &stdev);
void displayData(void);
void syncSample(void);
void configureArduino(void);
void ISR_Sample();

#endif