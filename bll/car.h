/*
 * car.h
 *
 * Created: 7/17/2018 8:37:27 PM
 *  Author: abdullah
 */ 

#ifndef _CAR_H_
#define _CAR_H_

#include <stdint.h>

typedef enum {
  NO_REQUEST = 0,
  REQUEST_FORWARD,
  REQUEST_BACKWARD,
  REQUEST_LEFT,
  REQUEST_RIGHT,
  REQUEST_STOP
} car_Requests;

 void car_initialize();
 void car_run(); 
 void car_setRequest(car_Requests req);
 uint8_t car_setSpeed(uint8_t percentage);
 
 
#endif // _CAR_H_
 