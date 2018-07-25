/*
* app.c
*
* Created: 7/17/2018 8:28:17 AM
* Author : abdullah
*/

#include <stdint.h>

#include "hal/interrupt.h"
#include "hal/sleep.h"

#include "bll/util.h"
#include "bll/uart.h"
#include "bll/car.h"
#include "bll/ultrasonic.h"
#include "bll/timer.h"

#define DISPLAY_INTERVAL 5000


inline void initialize() {
  timer_initialize();
  uart_initialize();
  ultrasonic_initalize();
  car_initialize();
}

int main(void) {
  initialize();
  hal_interrupt_enableGlobalInterrupt();
    
  uint16_t lastDisplayTickCount = 0;
 
  while(1) {
    timer_run();
    uart_run();
    ultrasonic_run();
    car_run();
    
    if(timer_getTickCount() - lastDisplayTickCount > DISPLAY_INTERVAL) {
      uart_sendString("Distance: ", 10);
      uart_sendNumber(ultrasonic_getDistanceInCm());
      uart_sendString(" cm.", 4);
      uart_sendLineBreak();
      
      lastDisplayTickCount = timer_getTickCount();
    }

    hal_sleep_enterSleepMode();
  }
}
