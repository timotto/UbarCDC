#include <AsyncMBus.h>
#include <TimerOne.h>
#include "settings.h"
#include "cdc.h"
#include "bt.h"

t_cdc_status cdc_status = {false, false, 1, 1, 0, 99, 4440};

void setup() {
  Serial.begin(57600);
  bt_setup();
  cdc_setup();
  DEBUG("[main] started\n");
}

uint32_t next = 0;
void loop() {
  uint32_t now = millis();
  if (now >= next) {
    next = now + 1000;
    if (cdc_status.playing)
      cdc_status.trackPosition++;
  }
  
  bt_loop();
  cdc_loop();
  
  while (Serial.available()) {
    char c = Serial.read();
    switch(c) {
      case 'a':
        cdc_active(true);
        break;
      case 'p':
        cdc_active(false);
        break;
      case 'r':
        cdc_reset();
        break;
    }
  }
}

bool btConnected = false;
bool btPlaying = false;

// Bluetooth callbacks
void bt_onConnection(bool connected) {
  btConnected = connected;
  btPlaying = false;
  if (connected && cdc_status.selected) {
    bt_sendAvcrp(BT_AVCRP_PLAY);
  }
}

void bt_onStreaming(bool streaming) {
  btPlaying = streaming;
}

void cdc_selected(bool selected) {
  cdc_status.selected = selected;
  if (selected) {
    if (btConnected && !btPlaying) {
      bt_sendAvcrp(BT_AVCRP_PLAY);
    }
  } else {
    if (btConnected && btPlaying) {
      bt_sendAvcrp(BT_AVCRP_PAUSE);
    }
  }
}

void cdc_onSelectCd(int cd) {
  bt_selectCd(cd);
  cdc_status.currentDisc = cd;
  cdc_status.currentTrack = 1;
  cdc_status.numtracks = 5 + random(5,15);
  cdc_status.disclength = 3600 + random(0,1000);
}

void cdc_onSelectTrack(int track) {
  bt_selectTrack(track);
  int n, key;
  if (track > cdc_status.currentTrack) {
    n = track - cdc_status.currentTrack;
    key = BT_AVCRP_NEXT;
  } else if (track < cdc_status.currentTrack) {
    n = cdc_status.currentTrack - track;
    key = BT_AVCRP_PREV;
  } else return;
  
  for (int i=0;i<n;i++)
    bt_sendAvcrp(key);
  
  cdc_status.currentTrack = track;
}

void cdc_onPlay() {
  cdc_status.playing = true;
  bt_sendAvcrp(BT_AVCRP_PLAY);
}

void cdc_onPause() {
  cdc_status.playing = false;
  bt_sendAvcrp(BT_AVCRP_PAUSE);
}

void cdc_onNext() {
  bt_sendAvcrp(BT_AVCRP_NEXT);
}

void cdc_onPrev() {
  bt_sendAvcrp(BT_AVCRP_PREV);
}

void cdc_onFwd(bool startStop) {
  bt_sendAvcrp(BT_AVCRP_RWDSTART);
}

void cdc_onRwd(bool startStop) {
  bt_sendAvcrp(BT_AVCRP_RWDSTOP);
}


