#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"

#define DEBUG(x...)  Serial.print(x)
//#define DEBUG //(x)

#define CDC_MBUS
//#define CDC_IBUS
//#define CDC_VW
#define CDC_BUS_TX  2
#define CDC_BUS_RX  3

//#define BT_RN52
//#define BT_OVC3860
#define BT_DUMMY
#define BT_TX  4
#define BT_RX  5

#endif // SETTINGS_H
