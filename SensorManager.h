#include "Globals.h"
#include "SensorEntry.h"
#include "SortedArray.h"

#pragma once

const int NUM_SENSOR_ENTRIES = 8;
const int NUM_SCHEDULED_EVENTS = NUM_SENSOR_ENTRIES*2;

typedef unsigned long Timestamp;

struct SensorScheduledEvent {
  Timestamp date;
  SensorMeasurementFunc func;
  SensorEntry *argument;    // arbitrary argument for the above function, but probably a SensorEntry
  
  // comparator for sorting
  bool operator < (SensorScheduledEvent &o) {
    return date < o.date;
  }
};


// Holds all important global data, including:
// - map from sensor ID to sensor reading function (basically a list of supported sensor types)
// - list of sensors installed
// - priority queue of scheduled sensor reading events
class SensorManager {
public:
  char boardName[24];
  char boardDescription[80];
  
  // array of info about what sensors are attached
  // an entry is unused if it's func is null
  SensorEntry sensorEntries[NUM_SENSOR_ENTRIES];

  // ring buffer for scheduling
  SortedArray<SensorScheduledEvent, NUM_SCHEDULED_EVENTS> events;
  
  // static map between sensor ID string and a function called to take measurements
  static const SensorFunctionMapEntry sensorFunctionMap[];

public:
  SensorManager() {
    // sensorEntries have a constructor
  }
  
  // storing and loading settings when powered off
  void writeToEEPROM();
  void readFromEEPROM();
  
  void removeEntry(int index);
  
  void printEntry(int index);
  void printEntries();
  
  // create an event scheduled at entry->msMeasurementPeriod milliseconds from now
  void schedule(SensorEntry *entry);
  void schedule(int index) { schedule(sensorEntries + index); }
  
  // overloaded method allows you to specify a time to avoid frequency drift
  void schedule(SensorEntry *entry, unsigned long msTimeToScheduleFrom);
  
  // when a sensor entry is changed or deleted, remove its events
  void removeEvents(int index);
};

