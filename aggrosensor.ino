//#include "SensorEntry.h"
//#include "SensorProfiles.h"

//#include "profiles/EnvTmp.cpp"
//#include "profiles/DS18B20.h"

////////////////////////////////////////////////////////////////////////////////////
// Globals for knowing what sensors are plugged into the arduino and on what pins
////////////////////////////////////////////////////////////////////////////////////

char gBoardName[24];
char gBoardDescription[80];

const int MAX_SENSOR_ID_LENGTH = 16;

// All the info for a sensor plugged into the arduino
struct SensorEntry {
  char sensorID[MAX_SENSOR_ID_LENGTH];  // the "class" type
  char label[16];                   // label applied to data from this sensor
  char pins[4];                         // some number of pins used by the sensor
                                        // (order is dictated the sensor's read function)
  unsigned long msMeasurementPeriod;  // measurement period in milliseconds
  unsigned long dateNextMeasurement;    // for scheduling

  SensorEntry() {
    setEmpty();
  }
  
  void setEmpty() {
    sensorID[0] = '\0';
  }
  
  boolean isEmpty() const {
    return sensorID[0] == '\0';
  }
  
  boolean isDisabled() const {
    return msMeasurementPeriod == 0;
  }
  
  void schedule(unsigned long now) {
    dateNextMeasurement = now + msMeasurementPeriod;
  }
  
  void packageDataMessage(float data) const {
    Serial.print("{data: {label:\'");
    Serial.print(label);
    Serial.print("\' datum:");
    Serial.print(data);
    Serial.println('}}');
  }
};

// pre-allocated array for sensors
// you know an entry is unused if it's sensorID is ""
const int NUM_SENSOR_ENTRIES = 16;
SensorEntry gSensorEntries[NUM_SENSOR_ENTRIES];

typedef int (*SensorMeasurementFunc)(const struct SensorEntry *config);

int DummySensorMeasurementFunc(const struct SensorEntry *config) { return 0; }

// how to read data from a sensor TYPE
struct SensorFunctionMapEntry {
  char sensorID[MAX_SENSOR_ID_LENGTH];
  SensorMeasurementFunc func;
};

int read_tmp_env(const struct SensorEntry *entry)
{
  char pinDigital = entry->pins[0];
  char pinAnalog = entry->pins[1];
  pinMode(pinDigital, OUTPUT);
  digitalWrite(pinAnalog, LOW); 
  digitalWrite(pinDigital, HIGH);
  delay(2); 
  float v_out = analogRead(pinAnalog); 
  digitalWrite(pinDigital, LOW);
  v_out *= .0048; 
  v_out *= 1000; 
  float temp = 0.0512 * v_out -20.5128;
  
  entry->packageDataMessage(temp);
  return 0;  //no error
}

SensorFunctionMapEntry gSensorFunctionMap[] = {
	{"TMP-ENV", read_tmp_env}
};

////////////////////////////////////////////////////////////////////////////////////
// string/json parsing functions 
////////////////////////////////////////////////////////////////////////////////////


const char gIgnoreChars[] = " \n\r\t:,{}";  // set of chars we ignore as white space

boolean isIgnoredChar(char c) {
  boolean isIgnored = false;
  for (const char *ignore = gIgnoreChars; *ignore != '\0'; ignore++) {
    if (c == *ignore) {
      isIgnored = true;
      break;
    }
  }
  return isIgnored;
}

// Find location of next char which isn't "white space"
char* eatWhiteSpace(char *buf) {
  char c = *buf;

  //always end on a null char
  while (c != '\0') {
    if (!isIgnoredChar(c)) return buf;
    // this current c counts as white space, so move on to the next buf position
    c = *++buf;
  }

  return buf;
}

// return the start and end of the next token, after ignoring white space
char* tokenize(char *buf, char **tokenEnd)
{
  char *start = eatWhiteSpace(buf);
  char *s = start;  // iterator for buf
  char c = *s;

  // iterate through s until we find a characer which is not in gIgnoredChars
  while (c != '\0' && !isIgnoredChar(c)) {
    c = *++s;
  }

  // found the end of the token, so write it if we were given a location to store it
  if (tokenEnd) *tokenEnd = s;
  return start;
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
  
  if(!strcmp(cmd, "status")) {
    Serial.print("Status Response\n");
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
    
    SensorMeasurementFunc func = getSensorMeasurementFunc(entry->sensorID);
    if (func == DummySensorMeasurementFunc) continue;
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
        if (ixCommandEnd == 0) return;  // ignore extra "end" chars left over from previous command
      
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

#if 0
// (code thoughts in progress)
void parse(char *buf)
{
  // chars which go on the stack: '"[
  const int MAXSTACK = 20;
  int ixTop = 0;
  char stack[MAXSTACK];
  
  struct DeviceEntry {char name[16], sensorID[16];} devices[4];
  int ixDevice = 0;
  
  const int MAX_FIELD_NAME = 16;
  char fieldName[MAX_FIELD_NAME];
  boolean curStringIsFieldName = true;
  
  char *tokenStart, *tokenEnd;
  char *s;
  char c = *buf;

  tokenStart = tokenize(buf, tokenEnd);
  c = *tokenStart;
  
  if (c == '[') {
    
  }
  
  if (c == '\'' || c == '\"') {  // start of string
    char *s = tokenEnd;
    tokenStart++;  // move past quote mark
    //find end string quote mark
    while (c != *s) {
      if (*s == '\0') {
        //error string didn't end
        return;
      }
      s++;
    }
     
    strncpy(getStringDest(curStringIsFieldName, devices[ixDevice]), tokenStart, s-1-tokenStart);
  }
  
  if (stack[ixTop] == '\'' || stack[ixTop] == '\"') {

    while (*s != stack[ixTop]) s++;  // this allows nulls in string...
    
    if (curStringIsFieldName) strncpy(fieldName, tokenStart, s-tokenStart);
      strncpy(tokenStart, fieldNameToLocation(
}
#endif 




