/*
* car.c
*
* Created: 7/17/2018 8:37:38 PM
*  Author: abdullah
*/

#include "../hal/car.h"

#include "common.h"
#include "ultrasonic.h"
#include "timer.h"
#include "uart.h"
#include "car.h"


#define TOTAL_WAIT_TIME_IN_MS 200
#define SAFE_FRONT_DISTANCE_IN_CM 25

volatile car_Requests request = NO_REQUEST;
car_Requests lastRequest = REQUEST_STOP;
uint16_t turnTickCount;
uint8_t isTurning = FALSE;

void car_initialize() {
  hal_car_initialize();
  car_setSpeed(100);
}

uint8_t car_setSpeed(uint8_t percentage) {
  if(percentage > 100) {
    percentage = 100;
  }
  
  uint8_t registerValue = (uint8_t)((uint16_t)percentage * (uint16_t)255 / (uint16_t)100);
  hal_car_setSpeed(registerValue);
  return percentage;
}

void _startTurning() {
  turnTickCount = timer_getTickCount();
  isTurning = TRUE;
}

uint8_t _isTurningComplete() {
  return (timer_getTickCount() - turnTickCount) > TOTAL_WAIT_TIME_IN_MS;
}

inline void _stop() {
  lastRequest = REQUEST_STOP;
  hal_car_stop();
}

inline void _forward() {
  lastRequest = REQUEST_FORWARD;
  hal_car_moveForward();
}

inline void _backward() {
  lastRequest = REQUEST_BACKWARD;
  hal_car_moveBackward();
}

inline void _left() {
  hal_car_turnLeft();
  _startTurning();
}

inline void _right() {
  hal_car_turnRight();
  _startTurning();
}


void processRequest() {
  switch(request) {
    case REQUEST_STOP:     _stop();     break;
    case REQUEST_FORWARD:  _forward();  break;
    case REQUEST_BACKWARD: _backward(); break;
    case REQUEST_LEFT:    _left();      break;
    case REQUEST_RIGHT:   _right();     break;
    case NO_REQUEST:
    default:
    break;
  }
  
  request = NO_REQUEST;
}

void car_setRequest(car_Requests req) {
  request = req;
}

void car_run() {
  // Always check for front clearance
  if (SAFE_FRONT_DISTANCE_IN_CM > ultrasonic_getDistanceInCm()) {
    if(REQUEST_FORWARD == lastRequest) {
      _stop();
      uart_sendString("Obstacle! in ", 13);
      uart_sendNumber(ultrasonic_getDistanceInCm());
      uart_sendString(" cm.", 4);
      uart_sendLineBreak();
    }
    
    if(REQUEST_FORWARD == request) {
      request = REQUEST_STOP;
    }
  }
  
  if(request != NO_REQUEST) {
    processRequest();
  }
  
  if(isTurning && _isTurningComplete()) {
    request = lastRequest;
    isTurning = FALSE;
  }
}