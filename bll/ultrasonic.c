/*
* ultrasonic.c
*
* Created: 7/19/2018 5:32:13 PM
*  Author: abdullah
*/

#include "../hal/ultrasonic.h"

#include "common.h"
#include "timer.h"
#include "ultrasonic.h"

#define MEASUREMENT_INTERVAL_IN_MS 100

uint8_t isCalculationDone = FALSE;

uint16_t lastCalculatedDistanceInCm = 0;
volatile uint16_t lastTimerValue = 0;
volatile uint8_t isEchoReceived = FALSE;
uint16_t lastMeasurementTickCount = 0;

void _onEchoReceived(uint16_t ticks) {
  lastTimerValue = ticks;
  isEchoReceived = TRUE;
}

void ultrasonic_initalize() {
  hal_ultrasonic_onEchoReceived = _onEchoReceived;
  hal_ultrasonic_initalize();
}

void ultrasonic_triggerMeasurement() {
  isCalculationDone = FALSE;
  isEchoReceived = FALSE;
  hal_ultrasonic_triggerMeasurement();
}

uint8_t ultrasonic_isResultReady() {
  return isCalculationDone;
}

uint16_t ultrasonic_getDistanceInCm() {
  return lastCalculatedDistanceInCm;
}

void ultrasonic_run() {
  // Trigger a measurement if interval reached
  if(timer_getTickCount() - lastMeasurementTickCount > MEASUREMENT_INTERVAL_IN_MS) {
    ultrasonic_triggerMeasurement();
    lastMeasurementTickCount = timer_getTickCount();
  }
  
  // Calculate the distance
  if(!isCalculationDone && isEchoReceived) {
    lastCalculatedDistanceInCm = lastTimerValue / 58;
    isCalculationDone = TRUE;
  }
}
