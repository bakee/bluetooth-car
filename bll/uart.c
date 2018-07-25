/*
* uart.c
*
* Created: 7/18/2018 12:19:45 AM
*  Author: abdullah
*/

#include "../hal/uart.h"

#include "common.h"
#include "buffer.h"
#include "util.h"
#include "car.h"
#include "uart.h"

#ifdef DEBUG
#define NEWLINE_CHAR ('\r')
#else
#define NEWLINE_CHAR ('\n')
#endif

volatile uint8_t isTransmitPending = FALSE;

#define RX_BUFFER_SIZE 32
DEFINE_GLOBAL_BUFFER(rxBuffer, RX_BUFFER_SIZE);

#define TX_BUFFER_SIZE 64
DEFINE_GLOBAL_BUFFER(txBuffer, RX_BUFFER_SIZE);

volatile uint8_t isCommandPending = FALSE;

// Private functions

void _processCommand() {
  char command[10];
  uint8_t i, data, len = 0;
  for (i = 0; i < 10; ++i) {
    buffer_takeCharFromBuffer(&rxBuffer, &data);
    if(data == NEWLINE_CHAR) {
      break;
    }
    command[i] = data;
    len++;
  }
  
  if (stringCompare("d1", command, 2)) {
    car_setRequest(REQUEST_FORWARD);
    uart_sendString("Forward", 7);
  } else if (stringCompare("d2", command, 2))  {
    car_setRequest(REQUEST_BACKWARD);
    uart_sendString("Backward", 8);
  } else if (stringCompare("d3", command, 2))  {
    car_setRequest(REQUEST_LEFT);
    uart_sendString("Left", 4);
  } else if (stringCompare("d4", command, 2))  {
    car_setRequest(REQUEST_RIGHT);
    uart_sendString("Right", 5);
  } else if (stringCompare("d0", command, 2))  {
    car_setRequest(REQUEST_STOP);
    uart_sendString("Stop", 4);
  } else if (stringCompare("s", command, 1))  {
    uint16_t speedValue = stringToNumber(command + 1);
    uint8_t actualSpeed = car_setSpeed(speedValue);
    uart_sendString("Speed: ", 7);
    uart_sendNumber(actualSpeed);
    uart_sendChar('%');
  } else {
    uart_sendString("Unknown", 7);
  }
  
  uart_sendLineBreak();
}

void _transmit() {
  uint8_t data;
  if(buffer_takeCharFromBuffer(&txBuffer, &data)) {
    hal_uart_sendByte(data);
  }
}

void _onByteReceived(uint8_t receivedByte) {
  buffer_putCharToBuffer(&rxBuffer, receivedByte);
  if(receivedByte == NEWLINE_CHAR) {
    isCommandPending = TRUE;
  }
}

inline void _setTransmitPending() {
  isTransmitPending = TRUE;
}
// End of private functions
void uart_sendString(const char* string, uint8_t length) {
  buffer_putStringToBuffer(&txBuffer, string, length);
  _setTransmitPending();
}

void uart_sendChar(char c) {
  buffer_putCharToBuffer(&txBuffer, c);
  _setTransmitPending();
}

void uart_sendNumber(uint16_t number) {
  char distanceString[5];
  uint8_t digitCount = numberToString(number, distanceString);
  uart_sendString(distanceString, digitCount);
}

void uart_sendLineBreak() {
  uart_sendChar(NEWLINE_CHAR);
}

void uart_run() {
  if(isTransmitPending) {
    _transmit();
    isTransmitPending = FALSE;
  }
  
  if(isCommandPending) {
    _processCommand();
    isCommandPending = FALSE;
  }
}

void uart_initialize() {
  hal_uart_onTransmitComplete = _transmit;
  hal_uart_onReceiveComplete = _onByteReceived;
  hal_uart_initialize();
}