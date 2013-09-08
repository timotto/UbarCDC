
// TODO: remember last state when turned off if radio unit / cdc do the same

//#define CDC_MBUS
#define CDC_IBUS
#define BT_RN52_LIB
//#define BT_RN52
//#define BT_OVC3860

#define PWR_BT      3

// Bluetooth UART, SoftwareSerial
//#define BT_RX       8
//#define BT_TX       9

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
  Serial.begin(9600);
  cdc_setup();
  bt_setup();
  bt_on();
//  Serial.println("[UCDC] started");
}

void loop() {
  cdc_loop();
  con_loop();
  bt_loop();
}

void onPause() {
  bt_disconnect();
  delay(200);
  bt_off();
}

void onResume() {
  bt_on();
  delay(200);
  bt_reconnect();
}

void cdc_setActive(bool a) {
  if (active == a)return;
  active = a;
  if (active) {
    Serial.println("[UCDC] active");
  } else {
    Serial.println("[UCDC] not active");
  }
}

void cdc_setPlaying(bool p) {
  if (playing == p)return;
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


