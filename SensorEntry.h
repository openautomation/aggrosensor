char gBoardName[24];
char gBoardDescription[80];

const int SENSOR_ID_MAX_LENGTH = 16;

struct SensorEntry {
	unsigned long msCollectionInterval;
	char dataName[16];
	char sensorID[SENSOR_ID_MAX_LENGTH];
	char pins[4];
};

const int NUM_SENSOR_ENTRIES = 16;
SensorEntry SensorEntry[NUM_SENSOR_ENTRIES];


