#include <AsyncMBus.h>
#include <TimerOne.h>
#include <Readline.h>

#define CDC_BUS_TX  9
#define CDC_BUS_RX  3
AsyncMBus mBus(CDC_BUS_RX, CDC_BUS_TX, onMbusMessage);

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
  Serial.begin(115200);
  bt_setup();
  mBus.setup();
}

void loop() {
  mBus.loop();
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


