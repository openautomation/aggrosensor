#pragma once

#include <Arduino.h>
#include <avr/eeprom.h>

// macros for writing status messages in JSON to serial
#define printFlash(s, type)        \
  Serial.print(F("{" type ":\'")); \
  Serial.print(F(s));              \
  Serial.print("\'}");             \

#define printlnFlash(s, type)      \
  Serial.print(F("{" type ":\'")); \
  Serial.print(F(s));              \
  Serial.print("\'}\n");           \

// Functions with print level built in.  Useful because some can be turned off in release.
// These could be functions, but we would need overloaded versions for regular string vs. string stored in flash,
// and Arduino IDE is not friendly to templated functions.
#define printError(s)     printFlash(s, "error")
#define printlnError(s)   printlnFlash(s, "error")
#define printDebug(s)     printFlash(s, "debug")
#define printlnDebug(s)   printlnFlash(s, "debug")
#define printStatus(s)    printFlash(s, "status")
#define printlnStatus(s)  printlnFlash(s, "status")


// The following functions return the number of bytes read/written as a convenience
int memcpyToEEPROM(int location, const void *data, int byteCount);
int strcpyToEEPROM(int location, const char *s);
int memcpyFromEEPROM(int location, void *data, int byteCount);
