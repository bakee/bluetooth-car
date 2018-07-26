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

typedef enum {
  CAR_STATE_STOPPED,
  CAR_STATE_TURNING_LEFT,
  CAR_STATE_TURNING_RIGHT,
  CAR_STATE_MOVING_FORWARD,
  CAR_STATE_TURNING_FORWARD_LEFT,
  CAR_STATE_TURNING_FORWARD_RIGHT,
  CAR_STATE_MOVING_BACKWARD,
  CAR_STATE_TURNING_BACKWARD_LEFT,
  CAR_STATE_TURNING_BACKWARD_RIGHT
} CarStates;

volatile car_Requests request = NO_REQUEST;
uint16_t turnTickCount;
CarStates _carState = CAR_STATE_STOPPED;

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

void _resetTurnTickCount() {
  turnTickCount = timer_getTickCount();
}

uint8_t _isTurnTickCountExpired() {
  return (timer_getTickCount() - turnTickCount) > TOTAL_WAIT_TIME_IN_MS;
}

inline void _stop() {
  hal_car_stopLeftMotor();
  hal_car_stopRightMotor();
  _carState = CAR_STATE_STOPPED;
}

inline void _forward() {
  hal_car_forwardLeftMotor();
  hal_car_forwardRightMotor();
  _carState = CAR_STATE_MOVING_FORWARD;
}

inline void _backward() {
  hal_car_backwardLeftMotor();
  hal_car_backwardRightMotor();
  _carState = CAR_STATE_MOVING_BACKWARD;
}

inline void _left() {
  switch(_carState) {
    case CAR_STATE_STOPPED:
    case CAR_STATE_TURNING_LEFT:
    case CAR_STATE_TURNING_RIGHT: {
      hal_car_forwardRightMotor();
      hal_car_backwardLeftMotor();
      _carState = CAR_STATE_TURNING_LEFT;
    }
    break;
    case CAR_STATE_MOVING_FORWARD:
    case CAR_STATE_TURNING_FORWARD_LEFT:
    case CAR_STATE_TURNING_FORWARD_RIGHT: {
      hal_car_stopLeftMotor();
      _carState = CAR_STATE_TURNING_FORWARD_LEFT;
    }
    break;
    case CAR_STATE_MOVING_BACKWARD:
    case CAR_STATE_TURNING_BACKWARD_LEFT:
    case CAR_STATE_TURNING_BACKWARD_RIGHT: {
      hal_car_stopLeftMotor();
      _carState = CAR_STATE_TURNING_BACKWARD_LEFT;
    }
    break;
  }

  _resetTurnTickCount();
}

inline void _right() {
  switch(_carState) {
    case CAR_STATE_STOPPED:
    case CAR_STATE_TURNING_LEFT:
    case CAR_STATE_TURNING_RIGHT: {
      hal_car_forwardLeftMotor();
      hal_car_backwardRightMotor();
      _carState = CAR_STATE_TURNING_RIGHT;
    }
    break;
    case CAR_STATE_MOVING_FORWARD:
    case CAR_STATE_TURNING_FORWARD_LEFT:
    case CAR_STATE_TURNING_FORWARD_RIGHT: {
      hal_car_stopRightMotor();
      _carState = CAR_STATE_TURNING_FORWARD_RIGHT;
    }
    break;
    case CAR_STATE_MOVING_BACKWARD:
    case CAR_STATE_TURNING_BACKWARD_LEFT:
    case CAR_STATE_TURNING_BACKWARD_RIGHT: {
      hal_car_stopRightMotor();
      _carState = CAR_STATE_TURNING_BACKWARD_RIGHT;
    }
    break;
  }

  _resetTurnTickCount();
}


inline void _handleRequest() {
  if(NO_REQUEST == request) {
    return;
  }
  
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

void _handleForwardObstacle() {
  _stop();
  uart_sendString("Obstacle! in ", 13);
  uart_sendNumber(ultrasonic_getDistanceInCm());
  uart_sendString(" cm.", 4);
  uart_sendLineBreak();
}

uint8_t _isTurnInProgress() {
  switch(_carState) {
    case CAR_STATE_TURNING_LEFT:
    case CAR_STATE_TURNING_RIGHT:
    case CAR_STATE_TURNING_FORWARD_LEFT:
    case CAR_STATE_TURNING_FORWARD_RIGHT:
    case CAR_STATE_TURNING_BACKWARD_LEFT:
    case CAR_STATE_TURNING_BACKWARD_RIGHT:
      return TRUE;
    default:
      return FALSE;
  }
}

inline void _handleTurnTimer() {
  if(_isTurnInProgress() && _isTurnTickCountExpired()) {
    switch(_carState) {
      case CAR_STATE_TURNING_FORWARD_LEFT:
      case CAR_STATE_TURNING_FORWARD_RIGHT: {
        _forward();
      }
      break;
      
      case CAR_STATE_TURNING_BACKWARD_LEFT:
      case CAR_STATE_TURNING_BACKWARD_RIGHT: {
        _backward();
      }
      break;
      
      case CAR_STATE_TURNING_LEFT:
      case CAR_STATE_TURNING_RIGHT:
      default: {
        _stop();
      }
      break;
    }
  }
}

inline void _checkObstacle() {
  if (SAFE_FRONT_DISTANCE_IN_CM > ultrasonic_getDistanceInCm()) {
    if(CAR_STATE_MOVING_FORWARD == _carState) {
      _handleForwardObstacle();
    }
    
    // With Obstacle ahead, if a pending request of forward movement is present
    // then ignore that request
    if(REQUEST_FORWARD == request) {
      request = NO_REQUEST;
    }
  }
}

void car_run() {
  _checkObstacle();  
  _handleTurnTimer();  
  _handleRequest();
}