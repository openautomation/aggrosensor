#include <Wire.h>
#include "SensorManager.h"
#include "SensorProfiles.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sensor Map
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// why is sensorFunctionMap a member of SensorManager?
const int NUM_SENSOR_TYPES = sizeof(SensorManager::sensorFunctionMap) / sizeof(SensorFunctionMapEntry);

// TODO figure out dummy vs. null when telling if an entry is empty or printable but errorneous
SensorMeasurementFunc funcFromSensorID(const char *sensorID) {
  for (int i = 0; i < NUM_SENSOR_TYPES; i++) {
    if (!strcmp(SensorManager::sensorFunctionMap[i].sensorID, sensorID)) {
      return SensorManager::sensorFunctionMap[i].func;
    }
  }
  return NULL;
}

const char* idFromSensorFunc(SensorMeasurementFunc func) {
  for (int i = 0; i < NUM_SENSOR_TYPES; i++) {
    if (SensorManager::sensorFunctionMap[i].func == func) {
      return SensorManager::sensorFunctionMap[i].sensorID;
    }
  }
  return "NONE";
}

//int DummySensorMeasurementFunc(const struct SensorEntry *config) { return 0; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SensorManager methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SensorManager::removeEntry(int index) {
  if(index < 0 || index >= NUM_SENSOR_ENTRIES) return;
  removeEvents(index);
  sensorEntries[index].setEmpty();
}

void SensorManager::printEntry(int index) {
  if(index < 0 || index >= NUM_SENSOR_ENTRIES) return;
  sensorEntries[index].print(index);
}

void SensorManager::schedule(SensorEntry *entry) {
  schedule(entry, millis());
}

void SensorManager::schedule(SensorEntry *entry, unsigned long msTimeToScheduleFrom) {
  if (entry->isEmpty() || entry->isDisabled()) return;  // entry is disabled

  if (events.isFull()) {
    printlnError("Scheduler full");
    return;
  }
  
  /*Serial.print("scheduling ");
  Serial.print(entry->label);
  Serial.print(" at time ");
  Serial.println(msTimeToScheduleFrom);
  */
  SensorScheduledEvent event;
  event.date = msTimeToScheduleFrom + entry->msMeasurementPeriod;
  event.func = entry->func;
  event.argument = entry;
  events.add(event);
}

void SensorManager::removeEvents(int index) {
    /*Serial.print("removeEvents(");
    Serial.print(index);
    Serial.println(')');*/
    for (int i = 0; i < events.size();) {
    // match the entry address
    if (events[i].argument == sensorEntries + index) events.remove(i);
    else i++;
  }
}

void SensorManager::printEntries() {
  Serial.print(F("{\"SensorEntries\":["));
  for (int i = 0; i < NUM_SENSOR_ENTRIES; i++) {
    if (sensorEntries[i].isEmpty()) continue;
    if (i != 0) Serial.print(',');
    printEntry(i);
  }
  Serial.print("]}\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EEPROM
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This struct is placed/checked at EEPROM location 0 to make sure EEPROM contains valid settings.
// When making changes to the EEPROM storage format, version number should be taken into account.
const char EEPROM_HEADER_STRING[] = {'A', 'g', 'r', 'o', 'S', 'A'};
const unsigned char EEPROM_MAJOR_VERSION = 0;
const unsigned char EEPROM_MINOR_VERSION = 0;

struct EEPROMHeader {
  char headerString[sizeof(EEPROM_HEADER_STRING)];
  unsigned short version;
  unsigned short numEntries;
  SensorEntry entries[0];
  
  EEPROMHeader() {
    memcpy(headerString, EEPROM_HEADER_STRING, sizeof(headerString));
    version = EEPROM_MAJOR_VERSION << 8 + EEPROM_MINOR_VERSION;  // little endian
    numEntries = 0;
  }
};

void SensorManager::writeToEEPROM() {
  EEPROMHeader h;
  int nextEmptyByte = offsetof(EEPROMHeader, entries);
  printlnDebug("AggroSensor writing settings to EEPROM...");

  // successively write every non-empty entry to eeprom and keep count
  for (int i = 0; i < NUM_SENSOR_ENTRIES; i++) {
    if (sensorEntries[i].isEmpty()) continue;
    nextEmptyByte += sensorEntries[i].writeToEEPROM(nextEmptyByte);
    h.numEntries++;
  }
  
  // store the header
  memcpyToEEPROM(0, &h, sizeof(h));
  Serial.print("{debug:\'AggroSensor settings (");
  Serial.print(h.numEntries);
  Serial.print(" entries) written to EEPROM.\'}\n");
}

void SensorManager::readFromEEPROM() {
  EEPROMHeader current, test;
  memcpyFromEEPROM(0, &current, sizeof(current));
  
  // check to see if valid settings are saved in EEPROM
  if (memcmp(current.headerString, test.headerString, sizeof(current.headerString))) {
    printlnDebug("EEPROM does not contain AggroSensor settings.");
    return;
  }

  // additionally, check for the current version
  if (current.version != test.version) {
    printlnError("AggroSensor settings stored in EEPROM have wrong version.");
    return;
  }
  
  int i = 0, nextByte = offsetof(EEPROMHeader, entries);
  for (; i < current.numEntries; i++) {
    nextByte += sensorEntries[i].readFromEEPROM(nextByte);
  }
  
  //clear the entries not written from eeprom
  int numEntriesRead = i;
  for(; i < NUM_SENSOR_ENTRIES; i++) {
    sensorEntries[i].setEmpty();
  }

  Serial.print("{status:\'AggroSensor settings (");
  Serial.print(numEntriesRead);
  Serial.print(" entries) loaded from EEPROM.\'}\n");
};

