// Define system header files and standard libraries used by Grbl
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <math.h>
#include <inttypes.h>    
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "SensorEntry.h"
#include "SensorProfiles.h"

#include "profiles/EnvTmp.h"
//#include "profiles/DS18B20.h"

#include "RingBuffer.h"
#include <JsonParser/JsonParser.h>
 
using namespace ArduinoJson::Parser;
/*
char json[] = "{\"sensor\":\"outdoor\",\"value\":25.6}";

    JsonParser<10> parser;
    JsonHashTable hashTable = parser.parseHashTable(json);
    
    Serial.println(hashTable.getString("sensor"));
    Serial.println(hashTable.getDouble("value"));
*/


struct SensorScheduledEvent {
  unsigned long dateEvent;
  SensorMeasurementFunc func;
  SensorEntry *argument;    // arbitrary argument for the above function
};

class EventScheduler {
  RingBuffer<SensorScheduledEvent, SensorEntry::NUM_SENSOR_ENTRIES*2> events;
  
public:
  EventScheduler() {
    
  }
};


void setEntry(char *buf)
{
  int index = -1;
  SensorEntry entryData;

  char *fieldEnd;
  char *fieldStart = tokenize(buf, &fieldEnd);
  if (fieldEnd) { continue; }
  *fieldEnd = '\0';
  char *valueStart;
  if (!strcmp(fieldStart, "index")) {
    index = atoi(tokenize(value));  // TODO: check that value is a nonnegative integer
  }
  else if (!strcmp(fieldStart, "label")) {
    if (fieldEnd - fieldStart > sizeof SensorEntry::label - 1) {
      //error label name too long (longer than 16)
      return;
    }
    entryData.label, tokenize(valueStart)
}

void processCommand(char *buf)
{
  char *cmdEnd = NULL;
  char *cmd = tokenize(buf, &cmdEnd);
  
  if (cmdEnd == NULL) {
    Serial.print("Something went wrong parsing command: ");
    Serial.println(buf);
    return;
  }
  
  // put a null after the token so we can use naive strcmp
  *cmdEnd = '\0';
  char *nextEnd = NULL;
  char *nextToken = tokenize(cmdEnd+1, &nextEnd);
  
  if(!strcmp(cmd, "entries")) {
    Serial.print("Status Response\n");
  }
  else if (!strcmp(cmd, "getEntry") {
    
  }
  else if (!strcmp(cmd, "setEntry") {
    setEntry(nextEnd);
  }
  else {
    Serial.print("Unknown Command\n");
  }
}

char bufCommand[80];
int ixCommandEnd = 0;
unsigned long msLastData = 0;

void setup()
{
  gSensorEntries[0].msMeasurementPeriod = 1000;
  strcpy(gSensorEntries[0].label, "upper temp");
  strcpy(gSensorEntries[0].sensorID, "TMP-ENV");
  gSensorEntries[0].pins[0] = 4;
  gSensorEntries[0].pins[1] = 13;

  Serial.begin(9600);
  Serial.println("Hello Terminal");  
  
  // take initial measurements and schedule the next ones
  for (int i = 0; i < NUM_SENSOR_ENTRIES; i++) {
    struct SensorEntry *entry = gSensorEntries + i;
    if (entry->isEmpty() || entry->isDisabled()) continue;
    
    if(gSensorEntries[i].measure()) {
      // error
    }
    gSensorEntries[i].schedule(millis());
  }
}

//read input from serial and parse messages
//update any changes to sensor read events
//setup timer events so you know when next to read from what sensor
void loop ()
{
  // take measurements
  for (int i = 0; i < NUM_SENSOR_ENTRIES; i++) {
    struct SensorEntry *entry = gSensorEntries + i;
    if (entry->isEmpty() || entry->isDisabled()) continue;
    if (millis() < entry->dateNextMeasurement) continue;

    // schedule first: add the measurement period to try not to drift
    entry->dateNextMeasurement += entry->msMeasurementPeriod;
    
    SensorMeasurementFunc func = entry->getSensorMeasurementFunc();
    if (func == DummySensorMeasurementFunc) {
      //error sensorID not found
      continue; 
    }
    if (func(entry)) {
      // error
    }
  }
  
  unsigned int numChars;  // always 1...
  if (numChars = Serial.available())
  {
      char c = Serial.read();
      
      // TODO handle buffer overflow better
      if (ixCommandEnd == sizeof(bufCommand)) {
        ixCommandEnd = 0;
        Serial.println("Error: command exceeded 80 chars");
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
  
  //simulate sending sensor data
  /*unsigned long msDataDelay = 1000;
  unsigned long ms = millis();
  if (ms > msLastData + msDataDelay) {
    Serial.print("{data:");
    Serial.print(ms);
    Serial.println('}');
    msLastData = ms;
  }*/
}
