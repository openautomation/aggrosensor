#include "SensorManager.h"

#include "EnvTmp.h"
//#include "profiles/DS18B20.h"

const SensorFunctionMapEntry SensorManager::sensorFunctionMap[] = {
  {"ENV-TMP", read_tmp_env}
};

