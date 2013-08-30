#define CDC_MBUS

// https://github.com/rabinath/Readline
#include <Readline.h>

#ifdef CDC_MBUS
// https://github.com/rabinath/AsyncMBus
#include <AsyncMBus.h>
// http://playground.arduino.cc/code/timer1
#include <TimerOne.h>
#define DISC_LAYOUT    0x000001575020ull
#define CDC_BUS_TX  7
#define CDC_BUS_RX  3
#endif

#define BT_RX       8
#define BT_TX       9
#define BT_GPIO2    2
#define BT_GPIO9    4

// The special disc was selected
bool active = false;
// Audio is playing
bool playing = false;
// Bluetooth profile connected
bool bt_iap = false;
bool bt_spp = false;
bool bt_a2dp = false;
bool bt_hfp = false;
// Bluetooth connection state
uint8_t bt_state = 0;

void setup() {
  Serial.begin(57600);
  bt_setup();
  cdc_setup();
  Serial.println("[UCDC] started");
}

void loop() {
  cdc_loop();
  con_loop();
  bt_loop();
}

void cdc_setActive(bool a) {
  active = a;
  if (active) {
    Serial.println("[UCDC] active");
  } else {
    Serial.println("[UCDC] not active");
  }
}

void cdc_setPlaying(bool p) {
  playing = p;
  if (playing) {
    Serial.println("[UCDC] playing");
    bt_play();
  } else {
    Serial.println("[UCDC] not playing");
    bt_pause();
  }
}

void bt_stateChanged() {
}


