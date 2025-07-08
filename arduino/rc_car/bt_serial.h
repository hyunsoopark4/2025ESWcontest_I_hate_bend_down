#ifndef BT_SERIAL_H
#define BT_SERIAL_H
#include <Arduino.h>

void printf_chunked(Stream &serial, const char *format, ...);

#endif
