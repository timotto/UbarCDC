#ifdef BT_OVC3860

// https://github.com/rabinath/Readline
#include <Readline.h>
#include "ConstStrings.h"

ReadLine btrl;

void bt_sppTx(uint8_t c) {}

void bt_setup(){
  Serial.begin(115200);
}

void bt_loop(){
  char *line;
  if ((line = btrl.feed(&Serial))) {
//    Serial.print("Line from BT: [");
//    Serial.print(line);
//    Serial.println("]");
  }
}

void bt_moduleSetup() {
}

void bt_play() {
  _bt_ovccmd(BT_O_CMD_PLAYPAUSE);
}

void bt_pause() {
  _bt_ovccmd(BT_O_CMD_STOP);
}

void bt_prev() {
  _bt_ovccmd(BT_O_CMD_PREV);
}

void bt_next() {
  _bt_ovccmd(BT_O_CMD_NEXT);
}

void bt_visible() {
  _bt_ovccmd(BT_O_CMD_PAIR_ON);
}

void bt_invisible() {
  _bt_ovccmd(BT_O_CMD_PAIR_OFF);
}

void bt_reconnect() {
  _bt_ovccmd(BT_O_CMD_RECONNECT);
}

void bt_disconnect() {
  _bt_ovccmd(BT_O_CMD_DISCONNECT);
}

void _bt_ovccmd(const char *cmd) {
  Serial.print(BT_O_CMD_PREFIX);
  Serial.print(cmd);
  Serial.print(BT_O_CMD_POSTFIX);
}

#endif

