//#define CDC_MBUS
#define CDC_IBUS

// Bluetooth UART, SoftwareSerial
#define BT_RX       8
#define BT_TX       9

// RN-52 specific PINs
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
// Bluetooth connection state (RN-52)
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


