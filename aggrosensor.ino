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

#include <Wire.h>
#include <JsonGenerator.h>
#include <JsonParser.h>

using namespace ArduinoJson::Parser;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO put in ifdef based on board type
const int A0_PIN_OFFSET = 14;  // arduino uno

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

// stores pin number in result, or -1 if pin number was empty
// returns false if an invalid pin number was provided
boolean getPinFromJson(ArduinoJson::Parser::JsonValue jsonValue, char &result)
{
  const char *sPin;
  if (jsonValue.success() && (sPin = jsonValue)) {
    // accept a nonnegative integer, possibly starting with 'A'
    if (sPin[0] >= '0' && sPin[0] <= '9') {
      result = atoi(sPin);
    }
    // which pin A0 corresponds to depends on board
    else if (sPin[0] == 'A') {
      result = atoi(sPin + 1) + A0_PIN_OFFSET;
    }
    else {
      printlnError("pins must be nonnegative integers, and may optionally begin with A");
      return false;
    }
  }
  else {
    result = -1;
  }
  
  return true;
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
    if (*sPeriod == '-' || entry.msMeasurementPeriod > 1209600000) {
      printlnError("msMeasurementPeriod must be between 0 and 1.2 billion milliseconds; 0 disables measurement");
      return;
    }
  }
  
  JsonValue pins = pairs["pins"];
  if (pins.success()) {
    if (pins[NUM_PINS].success()) {
      printlnError("only up to 4 pins are supported");
      return;
    }
    for (int i = 0; i < NUM_PINS; i++) {
      entry.pins[i] = -1;               //set unused pins to -1
      if (! getPinFromJson(pins[i], entry.pins[i])) {
        return;  // erroneous pin number? abort
      }
    }
  }
  
  // if something important changed, purge events previously scheduled and schedule new events  
  bool reschedule = (entry.msMeasurementPeriod != managerEntry.msMeasurementPeriod || entry.func != managerEntry.func);

  memcpy(&managerEntry, &entry, sizeof(entry));
  _manager.writeToEEPROM();

  if (reschedule)
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
    printlnStatus("setEntry succeeded");
  }
  else if (!strcmp(cmd, "setEntry")) {
    if (!param.isObject()) {
      printlnError("value of setEntry is not an object");
      return;
    }
    //Serial.println("setEntry");
    setEntry(param);
    printlnStatus("setEntry succeeded");
  }
  // example: {writeA: {pin:14, value:0}}
  else if (!strcmp(cmd, "writeA")) {
    JsonValue jsonPin = param["pin"];
    JsonValue jsonValue = param["value"];
    if (!jsonPin.success() || !jsonValue.success()) {
      printlnError("missing pin or value");
      return;
    }
    
    char pin;
    if (! getPinFromJson(jsonPin, pin)) return;
    if (pin == -1) {
      printlnError("pin number bad or missing");
      return;
    }
    
    // TODO: error check value
    const char *sValue = jsonValue;
    int value = atoi(sValue);
    
    pinMode(pin, OUTPUT);
    analogWrite(pin, value);
  }
  // TODO: combine digital and analog once they work
  else if (!strcmp(cmd, "writeD")) {
    JsonValue jsonPin = param["pin"];
    JsonValue jsonValue = param["value"];
    if (!jsonPin.success() || !jsonValue.success()) {
      printlnError("missing pin or value");
      return;
    }
    
    char pin;
    if (! getPinFromJson(jsonPin, pin)) return;
    if (pin == -1) {
      printlnError("pin number bad or missing");
      return;
    }
    
    // TODO: error check value
    const char *sValue = jsonValue;
    int value = atoi(sValue);
    
    pinMode(pin, OUTPUT);
    analogWrite(pin, value);
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

// finds the first whitespace character and sets it to null
// returns the index of the null character
int tokenizeOneWord(char *buf) {
  int i = -1;
  char c;
  do {
    i++;
    c = buf[i];
  } while(c != '\0' && c != ' ' && c != '\t' && c != '\n' && c != '\r');
  
  buf[i] = '\0';
  return i;
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
  ReferenceVoltage = 5.0;
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
