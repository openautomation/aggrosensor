#pragma once

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

#include "profiles/EnvTmp.h"
//#include "profiles/DS18B20.h"

SensorFunctionMapEntry SensorEntry::gSensorFunctionMap[] = {
	{"TMP-ENV", read_tmp_env}
};

//const int NUM_SENSOR_TYPES = sizeof SensorEntry::gSensorFunctionMap / sizeof SensorFunctionMapEntry;
