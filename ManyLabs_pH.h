#include "SensorEntry.h"

int read_ManyLabs_pH(const SensorEntry *entry) {
  // A0 = x48, A1 = x49, A2 = x4A, A3 = x4B,
  // A4 = x4C, A5 = x4D, A6 = x4E, A7 = x4F
  const int ADDRESS = 0x4D;
  const float opampGain = 5.25;
  const float pH7 = 0, pHStep = 1;
  
  Wire.begin(); //conects I2C
  
  //We'll assign 2 BYTES variables to capture the LSB and MSB(or Hi Low in this case)
  //We'll assemble the 2 in this variable
   
  Wire.requestFrom(ADDRESS, 2);        //requests 2 bytes
  while(Wire.available() < 2);         //while two bytes to receive
  byte adc_high = Wire.read();
  byte adc_low = Wire.read();
  int adc_result = (adc_high * 256) + adc_low;
  
  float milliVolts = (float)adc_result / 4096 * ReferenceVoltage * 1000;
  float milliVoltsForpH7 = (float)pH7 / 4096 * ReferenceVoltage * 1000;
  float pH = 7 - (milliVoltsForpH7 - milliVolts) / opampGain / pHStep;
  
  entry->packageDataMessage(pH);
  return 0;  //no error
}
