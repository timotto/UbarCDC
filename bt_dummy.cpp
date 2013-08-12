#include "settings.h"

#ifdef BT_DUMMY
#include "bt.h"

void bt_setup() {
  DEBUG("[BT] Using Dummy BT interface\n");
}

void bt_loop() {
}

void bt_sendAvcrp(int key) {
  DEBUG("[BT] sending key [");
  switch(key) {
    case BT_AVCRP_PLAY:
      DEBUG("play");
      break;
    case BT_AVCRP_PAUSE:
      DEBUG("pause");
      break;
    case BT_AVCRP_PREV:
      DEBUG("prev");
      break;
    case BT_AVCRP_NEXT:
      DEBUG("next");
      break;
    case BT_AVCRP_RWDSTART:
      DEBUG("rwd start");
      break;
    case BT_AVCRP_RWDSTOP:
      DEBUG("rwd stop");
      break;
    case BT_AVCRP_FWDSTART:
      DEBUG("ffwd start");
      break;
    case BT_AVCRP_FWDSTOP:
      DEBUG("ffwd stop");
      break;
    default:
      DEBUG("unknown");
      break;
  }
  DEBUG("]\n");
}

void bt_slectCd(int cd) {
  DEBUG("[BT] selecting CD ");DEBUG(cd);DEBUG(" via SPP\n");
}


#endif

