/* OneWire Sensors Library for Arduino
 * Version: 1.0 By: Kamots http://theforest.us/lab/
 * Copyright 2013 kamotswolf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************
 * Some parts based on OneWire library examples http://www.pjrc.com/teensy/td_libs_OneWire.html
 */
#include "OneWireSensors.h"
#include <OneWire.h> // version 2.1 of this library seems to work best

#if USE_FLOAT
int OneWireSensors::getOWTemps(OneWire::OneWire ds, float *temps, uint16_t *ids, int maxdevs) {
#else
int OneWireSensors::getOWTemps(OneWire::OneWire ds, int *temps, uint16_t *ids, int maxdevs) {
#endif
  DeviceType devT;
  byte present = 0;
  uint8_t addr[8];
  uint8_t data[9];
  uint8_t crc;
  byte loopcnt = 0;
  int devcnt = 0;
  boolean didsearch = false;
#if USE_FLOAT
  float celsius;
#else
  int32_t celsius;
#endif

  memset(temps, 0, maxdevs*sizeof(temps[0])); // Need to clear the arrays, otherwise you may get invaid/old data
  memset(ids, 0x0, maxdevs*sizeof(ids[0]));

  ds.reset();
  while(ds.search(addr)) {
    loopcnt++;
    if((loopcnt > 128) || (devcnt >= maxdevs)) { // Don't try to scan over 128 devices on 1-Wire Net, or exceed input arrays length
      ds.reset();
      ds.reset_search();
      return devcnt;
    }
    didsearch = true;
    if (ds.crc8(addr, 7) != addr[7]) {
      continue;
    }

    switch (addr[0]) {
#if SUPPORT_DS18X20
      case 0x10: // DS18S20, DS1820, DS1920
        devT = DS18S20_1820;
        break;
      case 0x28: // DS18B20
        devT = DS18B20_1822;
        break;
      case 0x22: // DS1822
        devT = DS18B20_1822;
        break;
#endif
#if SUPPORT_IBUTTON
	  case 0x21: // DS1921G, DS1921H, DS1921Z (only DS1921G has been tested)
        devT = DS1921G;
        break;
      case 0x41: // DS1923, DS1922L, DS1922T, DS1922E
        ds.reset();
        ds.select(addr);
        ds.write_bytes((uint8_t[]){0x69, 0x26, 0x02}, 3, false); // Read Address 0226 (device type)
        for (int i = 0; i < 8; i++) ds.write(0xFF);
        ds.read_bytes(data, 9);
        if (data[0] == 64) { // DS1922L
          devT = DS1922L_1923;
        } else if (data[0] == 32) { // DS1923
          devT = DS1922L_1923;
        } else {
          devT = UNSUPPORTED; // Others in this series haven't been tested
        }
        break;
#endif
      default:
        devT = UNSUPPORTED;
    }
    if (devT == UNSUPPORTED) {
      ds.reset();
      continue;
    }

    ds.reset();
    ds.select(addr);
#if SUPPORT_IBUTTON
    if (devT == DS1922L_1923) {
      ds.write_bytes((uint8_t[]){0x69, 0x12, 0x02}, 3, false); // Request configuration data
      for (int i = 0; i < 8; i++) ds.write(0xFF);
      ds.read_bytes(data, 9);
      ds.reset();
      if (data[0] == 1 || data[0] == 3) {
        ds.select(addr);
        ds.write(0x55);
        ds.write(0xFF,1);
      } else { ds.reset(); continue; } // DO NOT run conversion if RTC not enabled! Will brick the iButton!
    } else {
#endif
#if SUPPORT_DS18X20
      if (devT == DS18B20_1822) {
        ds.write(0xBE); // Get scratchpad, which includes config data
        ds.read_bytes(data, 9);
        ds.reset();
        ds.select(addr);
        if(data[4] != 0x1F) { // Check if resolution isn't 9 bits
          ds.write_bytes((uint8_t[]){0x4E,0x50,0x05,0x1F,0x48}, 5, true); // Change resolution to 9 bits
          ds.reset();
          ds.select(addr);
        }
      }
#endif
      ds.write(0x44,1);
#if SUPPORT_IBUTTON
	}
#endif

#if SUPPORT_IBUTTON
    if (devT == DS1922L_1923) {
      delay(600); // Must wait this long for full resolution conversion
    } else {
#endif
	  delay(100); // 9 bit resolution conversion delay
#if SUPPORT_IBUTTON
	}
#endif

    present = ds.reset();
    ds.select(addr);
    if (present) {
#if SUPPORT_IBUTTON
      if (devT == DS1921G) {
        ds.write_bytes((uint8_t[]){0xF0, 0x11, 0x02}, 3); // Command to get memory with temperature data
      } else if (devT == DS1922L_1923) {
        ds.write_bytes((uint8_t[]){0x69, 0x0C, 0x02}, 3); // Command to get memory with temperature data
        for (int i = 0; i < 8; i++) ds.write(0xFF);
      } else {
#endif
        ds.write(0xBE); // Command to get scratchpad, which includes temperature data
#if SUPPORT_IBUTTON
      }
#endif
    } else { ds.reset(); continue; }
      
    ds.read_bytes(data, 9); // Read temperature data

#if SUPPORT_IBUTTON
    if (devT == DS1921G || devT == DS1922L_1923) {
      crc = data[8]; // TODO: should be a call to crc16 with the full data
    } else {
#endif
      crc = OneWire::crc8(data, 8);
#if SUPPORT_IBUTTON
    }
#endif

    if (data[8] == crc) { // Verify data
      switch (devT) {
#if USE_FLOAT
#if SUPPORT_IBUTTON
        case DS1922L_1923:
          celsius = (((float)data[1] / 2.0f) - 41.0f) + ((float)data[0] / 512.0f);
          break;
        case DS1921G:
          celsius = ((float)data[0] / 2.0f) - 40.0f;
          break;
#endif
        default:
#if SUPPORT_DS18X20
		  celsius = (float)((data[1] << 8) | data[0]) / 16.0;
#else
		  celsius = 0;
#endif
#else
#if SUPPORT_IBUTTON
        case DS1922L_1923:
          celsius = ((((int32_t)data[1] * 1000) / 2) - 41000) + (((int32_t)data[0] * 1000) / 512);
          break;
        case DS1921G:
          celsius = (((int32_t)data[0] * 1000) / 2) - 40000;
          break;
#endif
        default:
#if SUPPORT_DS18X20
          celsius = ((int32_t)((data[1] << 8) | data[0]) * 1000) / 16;
#else
		  celsius = 0;
#endif
#endif
}
#if USE_FLOAT
      temps[devcnt] = celsius;
#else
      temps[devcnt] = (int)(celsius / 1000);
#endif
      ids[devcnt] = (addr[0] << 8) | addr[7];
      devcnt++;
    }
  }
  ds.reset_search();
  return devcnt;
}