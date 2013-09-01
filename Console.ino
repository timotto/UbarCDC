// https://github.com/rabinath/Readline
#include <Readline.h>

ReadLine con_readLine;
void con_loop() {
  char *line;

  if ((line = con_readLine.feed(&Serial))) {
//    Serial.print("CON> ");
//    Serial.print(line);
    if (0 == strcmp("setup", line)) {
      bt_moduleSetup();
    } else if (0 == strcmp("visible", line)) {
      bt_visible();
    } else if (0 == strcmp("invisible", line)) {
      bt_invisible();
    } else if (0 == strcmp("disconnect", line)) {
    } else if (0 == strcmp("reconnect", line)) {
      bt_reconnect();
    } else if (0 == strcmp("play", line)) {
      bt_play();
    } else if (0 == strcmp("pause", line)) {
      bt_pause();
    } else if (0 == strcmp("prev", line)) {
      bt_prev();
    } else if (0 == strcmp("next", line)) {
      bt_next();
#ifdef CDC_MBUS
    } else if (0 == strncmp("mbus:", line, 5)) {
      injectMbus(line+5);
    } else if (0 == strncmp("cdc:", line, 4)) {
      injectCDC(line+4);
#endif
    } else {
      return;
//      Serial.println(" ERROR");
    }
//    Serial.println(" OK");
  }
}


