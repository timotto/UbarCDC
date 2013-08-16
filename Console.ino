ReadLine con_readLine;
void con_loop() {
  char *line;
  if (line = con_readLine.feed(&Serial)) {
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
    } else if (0 == strncmp("mbus:", line, 5)) {
      injectMbus(line+5);
    } else if (0 == strncmp("cdc:", line, 4)) {
      injectCDC(line+4);
    } else return;
    
    Serial.print("CON> ");
    Serial.println(line);
  }
}

void injectMbus(char *line) {
  uint64_t msg;
  int len;
  if(parseuint64t(line, &msg, &len))
    mBus.tx(msg);
}

void injectCDC(char *line) {
  uint64_t msg;
  int len;
  if(parseuint64t(line, &msg, &len))
    onMbusMessage(msg, len);
}

bool parseuint64t(char *line, uint64_t *msg, int *len) {
  int n = strlen(line);
  *len = n * 4;

  *msg = 0;
  for(int i=0;i<*len;i+=4) {
    int idx = n - (i/4) - 1;
    uint8_t val = line[idx];
    if (val >= '0' && val <= '9') {
      val = val - '0';
    } else if (val >= 'a' && val <= 'f') {
      val = val - 'a' + 10;
    } else if (val >= 'A' && val <= 'F') {
      val = val - 'A' + 10;
    } else return false;
    
    *msg |= (uint64_t)val<<i;
  }
  return true;
}


