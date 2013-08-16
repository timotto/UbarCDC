#ifndef BT_H
#define BT_H

#include "settings.h"

#define BT_AVCRP_PLAY       1
#define BT_AVCRP_PAUSE      2
#define BT_AVCRP_PREV       3
#define BT_AVCRP_NEXT       4
#define BT_AVCRP_RWDSTART   5
#define BT_AVCRP_RWDSTOP    6
#define BT_AVCRP_FWDSTART   7
#define BT_AVCRP_FWDSTOP    8

void bt_setup();
void bt_loop();
void bt_onConnection(bool connected);
void bt_onStreaming(bool streaming);
void bt_sendAvcrp(int key);
void bt_selectCd(int cd);
void bt_selectTrack(int track);

#endif // BT_H

