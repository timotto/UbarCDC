#include <MBus.h>
#include "settings.h"
#include "cdc.h"
#include "bt.h"
#include "api.h"

t_cdc_status cdc_status;

void setup() {
  Serial.begin(57600);
  pinMode(13, OUTPUT);
  delay(5000);
  digitalWrite(13, HIGH);
  
  bt_setup();
  cdc_setup();
  DEBUG("[main] started\n");
}

void loop() {
  bt_loop();
  cdc_loop();
}

bool cdcSelected = false;
bool btConnected = false;
bool btPlaying = false;

// Bluetooth callbacks
void bt_onConnection(bool connected) {
  btConnected = connected;
  btPlaying = false;
  if (connected && cdcSelected) {
    bt_sendAvcrp(BT_AVCRP_PLAY);
  }
}

void bt_onStreaming(bool streaming) {
  btPlaying = streaming;
}

// here begins the 'API'
// this implementation uses Bluetooth, change it here to use a Music Shield with SD & MP3
void cdc_selected(bool selected) {
  cdcSelected = selected;
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
  bt_slectCd(cd);
}

void cdc_onPlay() {
  bt_sendAvcrp(BT_AVCRP_PLAY);
}

void cdc_onPause() {
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


