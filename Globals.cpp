#include "Globals.h"

/*void printError(const __FlashStringHelper *s) {
  Serial.print(F("{error: "));
  Serial.print(s);
  Serial.print('}');
}

void printError(const char *s) {
  Serial.print(F("{error: "));
  Serial.print(s);
  Serial.print('}');
}

void printDebug(const __FlashStringHelper *s) {
  Serial.print(F("{debug: "));
  Serial.print(s);
  Serial.print('}');
}

void printDebug(const char *s) {
  Serial.print(F("{debug: "));
  Serial.print(s);
  Serial.print('}');
}

//void printlnError(const __FlashStringHelper *s) { Serial.print(s); Serial.print('\n'); }
//void printlnError(const char *s)                { Serial.print(s); Serial.print('\n'); }
void printlnDebug(const __FlashStringHelper *s) { Serial.print(s); Serial.print('\n'); }
void printlnDebug(const char *s)                { Serial.print(s); Serial.print('\n'); }
*/
int memcpyToEEPROM(int location, const void *data, int byteCount) {
  const char *p = (const char*)data;
  unsigned char *address = (unsigned char*)location;
  for(int i = 0; i < byteCount; i++) eeprom_write_byte(address+i, p[i]);
  return byteCount;
}

int strcpyToEEPROM(int location, const char *s) {
  unsigned char *address = (unsigned char*)location;
  int i;
  for(i = 0; s[i] != '\0'; i++) eeprom_write_byte(address+i, s[i]);
  eeprom_write_byte(address+i, '\0');
  return i;
}

int memcpyFromEEPROM(int location, void *data, int byteCount) {
  char *p = (char*)data;
  unsigned char *address = (unsigned char*)location;
  for(int i = 0; i < byteCount; i++) p[i] = eeprom_read_byte(address+i);
  return byteCount;
}
