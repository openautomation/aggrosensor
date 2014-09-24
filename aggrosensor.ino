// Define system header files and standard libraries used by Grbl
/*#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <math.h>
#include <inttypes.h>    
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>*/

#include "Globals.h"
#include "SensorEntry.h"
#include "SensorManager.h"

#include <JsonGenerator.h>
#include <JsonParser.h>

using namespace ArduinoJson::Parser;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////////////////////////////////

SensorManager _manager;

// serial input
char bufCommand[120];
int ixCommandEnd = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void measureAndSchedule(int index) {
  struct SensorEntry *entry = _manager.sensorEntries + index;
  if (entry->isEmpty() || entry->isDisabled()) return;
  
  if(entry->func(entry)) {
    // error
  }
  _manager.schedule(entry);
}


void setEntry(ArduinoJson::Parser::JsonValue &pairs)
{
  // index is required
  const char *sIndex = pairs["index"];
  int index = atoi(sIndex);
  if (*sIndex < '0' || *sIndex > '9' || index < 0 || index >= NUM_SENSOR_ENTRIES) {
    printlnError("index must be 0-15");
    return;
  }
  
  // copy the current entry settings, overwrite any fields specified, then copy it back
  SensorEntry entry;
  SensorEntry &managerEntry = _manager.sensorEntries[index];
  memcpy(&entry, &managerEntry, sizeof(entry));
  
  JsonValue label = pairs["label"];
  if (label.success()) {
    if (strlen(label) >= sizeof(entry.label)) {
      printlnError("label too long");
      return;
    }
    strcpy(entry.label, label);
  }
  
  JsonValue sensorID = pairs["sensorID"];
  if (sensorID.success()) {
    entry.func = funcFromSensorID((char*)sensorID);
    if (entry.func == NULL) {
      printlnError("sensorID not recognized");
      return;
    }
  }
  
  JsonValue period = pairs["msMeasurementPeriod"];
  if (period.success()) {
    const char *sPeriod = (char*)period;
    entry.msMeasurementPeriod = atol(sPeriod);
    if (*sPeriod == '-' || entry.msMeasurementPeriod == 0) {
      printlnError("msMeasurementPeriod must be between 1 and 4 billion");
      return;
    }
  }
  
  JsonValue pins = pairs["pins"];
  if (pins.success()) {
    for (int i = 0; i < NUM_PINS; i++) {
      JsonValue pin = pins[i];
      if (pin.success()) entry.pins[i] = (char)(long)pin;
      else entry.pins[i] = -1;      //set unused pins to -1
    }
  }

  memcpy(&managerEntry, &entry, sizeof(entry));
  _manager.writeToEEPROM();

  // if something important changed, purge events previously scheduled and schedule new events  
  if (entry.msMeasurementPeriod != managerEntry.msMeasurementPeriod ||
      entry.func != managerEntry.func)
  {
    _manager.removeEvents(index);
    measureAndSchedule(index);
  }
}

// Take a JSON object as a string, and parse the command name, and run the corresponding function
// Example JSON: "{command: {argument1: 1, argument2: '2'}}"
void processCommand(char *buf)
{
  // parses JSON 'inline' by modifying buf
  // the next line can easily cause stack overflow
  JsonParser<16> parser;
  JsonObject root = parser.parse(buf);
  if (!root.success()) {
    printlnError("parsing json");
    return;
  }
  
  JsonObjectIterator it = root.begin();
  const char *cmd = it.key();
  JsonValue param = it.value();
  
  if(!strcmp(cmd, "entries")) {
    //Serial.println("entries");
    _manager.printEntries();
  }
  else if (!strcmp(cmd, "getEntry")) {
    //Serial.println("getEntry");
    int index = int(it.value());
    _manager.printEntry(index);
    Serial.print('\n');
  }
  else if (!strcmp(cmd, "removeEntry")) {
    int index = int(it.value());
    _manager.removeEntry(index);
  }
  else if (!strcmp(cmd, "setEntry")) {
    if (!param.isObject()) {
      printlnError("value of setEntry is not an object");
      return;
    }
    //Serial.println("setEntry");
    setEntry(param);
  }
  else {
    printlnError("Unknown Command");
  }
}

void runScheduledEvents()
{
  // process all events whose timestamps have passed since the last time this function ran
  for (int index = 0; index < _manager.events.size(); index++) {
    
    // Events are sorted ascending by timestamp, so the first event is the 'soonest' to happen.
    // But if we are nearing clock overflow, events could have timestamps are WAY in the past; we'll skip those events.
    
    // See if the event's time has passed the current time.
    // The internal timer overflows after 50 days, but we can handle this if we assume events aren't scheduled more than 2 weeks in advance.
    // When the timer is nearing overflow, an event's time may overflow, making the current time MUCH LARGER than the event's time.
    long msSinceEvent = millis() - _manager.events[index].date;
    const long msTwoWeeks = 14 * 24 * 3600 * 1000 + 1;    // number of milliseconds in 2 weeks + 1 ms; must fit in signed long
    if (msSinceEvent < 0) break;      // event hasn't happened yet; don't need to examine the rest because they are later than this one
    if (msSinceEvent > msTwoWeeks) {  // event happened "so long ago" that the timer must be nearing overflow
      continue;                       // continue looking for an event that either just happened or has yet to happen
    }
    
    // The event's time has come/passed. Remove the event from the queue.
    SensorScheduledEvent event;
    _manager.events.remove(index, &event);
    struct SensorEntry *entry = event.argument;
    SensorMeasurementFunc func = entry->getMeasurementFunc();
  
    // Check constraints on the sensor entry.
    // The sensor may have been disabled or deleted since this event was scheduled.
    // TODO when a sensorEntry is modified, remove its currently scheduled events, and simplify this line
    if (entry->isEmpty() || entry->isDisabled() || func == NULL || entry->func != func) continue;
    
    // schedule the next measurement: add the measurement period to prevent drift
    _manager.schedule(entry, event.date);
  
    // call the measurement function  
    if (func(entry)) {
      // error
    }
  }
}

void handleSerialInput()
{
  unsigned int numChars;  // always 1...
  if (numChars = Serial.available())
  {
      char c = Serial.read();
      
      // TODO handle buffer overflow better
      if (ixCommandEnd == sizeof(bufCommand)) {
        ixCommandEnd = 0;
        printlnError("command exceeded 255 chars");
      }
      
      // assume end-of-command on carriage return or null terminator
      if (c == '\n' || c == '\r' || c == '\0') {
        if (ixCommandEnd == 0) return;  // ignore empty lines
      
        // null-terminate and process the command
        bufCommand[ixCommandEnd] = '\0';
        processCommand(bufCommand);
        ixCommandEnd = 0;
      }
      else {
        bufCommand[ixCommandEnd++] = c;
      }
  }
}


void createDebugEntries() {
  SensorEntry &entry = _manager.sensorEntries[0];
  entry.msMeasurementPeriod = 1000;
  strcpy(entry.label, "upper temp");
  entry.func = funcFromSensorID("ENV-TMP");
  entry.pins[0] = 2;
  entry.pins[1] = 14;
}

void setup()
{
  analogReference(DEFAULT);
  Serial.begin(38400);
  printlnStatus("AggroSensor version 0 started");

  createDebugEntries();
  _manager.readFromEEPROM();
  
  // take initial measurements and schedule the next ones
  for (int i = 0; i < NUM_SENSOR_ENTRIES; i++) {
    measureAndSchedule(i);
  }
}

//read input from serial and parse messages
//update any changes to sensor read events
//set timer events so you know when next to read from what sensor
void loop ()
{
  runScheduledEvents();
  handleSerialInput();
}
