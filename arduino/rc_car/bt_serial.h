#ifndef BT_SERIAL_H
#define BT_SERIAL_H
#include <Arduino.h>
#include <NeoSWSerial.h> // NeoSWSerial 타입을 사용하기 위해 포함

#define BUFFER_DELAY 50

void printf_chunked(Stream &serial, const char *format, ...);

 // BTserial is defined in bt_serial.cpp. This 'extern' allows other files to use it.
extern NeoSWSerial BTserial;

#endif
