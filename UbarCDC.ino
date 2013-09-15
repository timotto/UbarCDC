//#define DEBUG(x) Serial.print(x)
#define DEBUG(x)

#define CDC_IBUS
#define BT_RN52_LIB
#define SPP_JSON
//#define CDC_MBUS
//#define BT_RN52
//#define BT_OVC3860

// Bluetooth UART, SoftwareSerial
//#define BT_RX       8
//#define BT_TX       9

// RN-52 specific PINs
#define BT_GPIO2    2
#define BT_GPIO9    4

// The special disc was selected
bool active = true;
// Audio is playing
bool playing = true;
// Bluetooth profile connected
bool bt_iap = false;
bool bt_spp = false;
bool bt_a2dp = false;
bool bt_hfp = false;
// Bluetooth connection state (RN-52)
uint8_t bt_state = 0;

char artist[32] = {0};
char title[32] = {0};
char album[32] = {0};

void setup() {
  pinMode(3, INPUT);
  digitalWrite(3, HIGH);
  Serial.begin(9600);
  cdc_setup();
  bt_setup();
  onResume();
  DEBUG("[UCDC] started\n");
}

void loop() {

//  con_loop(); // Serial is owned by Bluetooth now
  cdc_loop();
  bt_loop();
  
}

uint8_t MSG_HELLO[] = {0xb0,0x05,0x0b,0x51,0x11,0x11,0xef};
uint8_t MSG_BYE[] = {0xbe, 0x04, 0xaf, 0x11, 0x11, 0x15};

/**
 * e.g. ignition is turned off, radio unit shuts down
 */
void onPause() {
  bt_disconnect();
}

/**
 * e.g. ignition is turned on, radio unit starts up
 */
void onResume() {
  bt_reconnect();
}

/**
 * UbarCDC has been chosen for audio output by the radio unit
 * or not (bool a)
 */
void cdc_setActive(bool a) {
  if (active == a)return;
  active = a;
  if (active) {
    DEBUG("[UCDC] active\n");
  } else {
    DEBUG("[UCDC] not active\n");
  }
}

/**
 * UbarCDC has been chosen to start playing music
 * or not (bool p)
 */
void cdc_setPlaying(bool p) {
  if (playing == p)return;
  playing = p;
  if (playing) {
    DEBUG("[UCDC] playing\n");
    bt_play();
  } else {
    DEBUG("[UCDC] not playing\n");
    bt_pause();
  }
}

void bt_stateChanged() {
}


