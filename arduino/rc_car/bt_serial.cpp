#include <Arduino.h>
#include <stdarg.h>

#define BUFFER_DELAY 50

void printf_chunked(Stream &serial, const char *format, ...)
{
    constexpr size_t chunksize = 20;
    constexpr unsigned int delayMs = 50;

    char sendBuffer[100];
    va_list args;
    va_start(args, format);
    vsnprintf(sendBuffer, sizeof(sendBuffer), format, args);
    va_end(args);

    size_t len = strlen(sendBuffer);
    size_t sent = 0;
    while (sent < len)
    {
        size_t toSend = (len - sent) < chunksize ? (len - sent) : chunksize;
        serial.write((const uint8_t *)(sendBuffer + sent), toSend);
        sent += toSend;
        delay(BUFFER_DELAY);
    }
}
