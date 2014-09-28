// Add your own measurement function and sensor IDs here.
// Make sure to include the header file that contains the measurement function.
// This is not a traditional header file; it's intended to be included at the top of SensorManager.cpp.

//#include "SensorManager.h"

#include "EnvTmp.h"
#include "DFRobot_pH.h"
#include "ManyLabs_pH.h"
//#include "profiles/DS18B20.h"

const SensorFunctionMapEntry SensorManager::sensorFunctionMap[] = {
  {"ENV-TMP",         read_tmp_env},
  {"DFROBOT-PH",      read_DFRobot_pH_Sensor},
  {"MANYLABS-PH",     read_ManyLabs_pH}
};

