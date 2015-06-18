#ifndef OneWireSensors_h
#define OneWireSensors_h

#include <OneWire.h>

#define USE_FLOAT 0 // Set to 1 to use floating point temperatures, uses about 814 more bytes of program space
// MUST have one or both of the below SUPPORT defines set to 1
#define SUPPORT_DS18X20 1 // Set to 0 if you don't use any DS18B20, DS1822, DS18S20, DS1820 or DS1920 sensors (214 bytes)
#define SUPPORT_IBUTTON 0 // Set to 1 if you use iButton-based sensors like DS1921, DS1922 or DS1923 (578 bytes)

class OneWireSensors
{
public:
#if USE_FLOAT
	int getOWTemps(OneWire::OneWire ds, float *temps, uint16_t *ids, int maxdevs);
#else
	int getOWTemps(OneWire::OneWire ds, int *temps, uint16_t *ids, int maxdevs);
#endif
	typedef enum DeviceType { DS18S20_1820, DS18B20_1822, DS1921G, DS1922L_1923, UNSUPPORTED };
};
#endif