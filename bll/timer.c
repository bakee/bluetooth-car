/*
* timer.c
*
* Created: 7/19/2018 7:42:39 PM
*  Author: abdullah
*/

#include "../hal/timer.h"
#include "timer.h"


volatile uint16_t totalMillisecond = 0;
volatile uint16_t totalMicrosecond = 0;

void _updateTime(uint16_t msPerTick, uint16_t usPerTick) {
  totalMillisecond += msPerTick;
  totalMicrosecond += usPerTick;
}

void timer_initialize() {
  hal_timer_onTimer0Overflow = _updateTime;
  hal_timer0_initialize();
}

uint16_t timer_getTickCount() {
  return totalMillisecond;
}

void timer_run() {
  // Adjust microseconds
  if(totalMicrosecond > 1000) {
    totalMillisecond += (totalMicrosecond / 1000);
    totalMicrosecond %= 1000;
  }
}