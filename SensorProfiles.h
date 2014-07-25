#include "SensorEntry.h"

#include "profiles/EnvTmp.h"
//#include "profiles/DS18B20.h"

int read_tmp_env(SensorEntry *config) {

	packageDataMessage();
}

struct SensorFunctionMapEntry {
  char sensorID[SENSOR_ID_MAX_LENGTH];
  int (*func)(SensorEntry *config);
};

SensorFunctionMapEntry gSensorFunctionMap[] = {
	{"TMP-ENV", read_Tmp_Env}

};
