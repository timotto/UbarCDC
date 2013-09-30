#ifdef BT_RN52_LIB

// https://github.com/rabinath/RN52lib
#include <RN52.h>
#include "ConstStrings.h"

class RNimpl : public RN52::RN52driver {
  void onProfileChange(BtProfile profile, bool connected);
public:
  void toUART(const char* c, int len);
  void fromSPP(const char* c, int len);
  void setMode(Mode mode);
  void onGPIO2();
  void moduleSetup();
//  void onError(int location, Error error) {
//    Serial.print("BT ERROR ");
//    Serial.print(error);
//    Serial.print(" @ ");
//    Serial.print(location);
//    Serial.print("\r");
//    
//    // Halt on error:
//    while(1) {
//      for(int i=0;i<location;i++) {
//        digitalWrite(13, HIGH);
//        delay(1500);
//        digitalWrite(13, LOW);
//        delay(500);
//      }
//      delay(1000);
//      for(int i=0;i<error;i++) {
//        digitalWrite(13, HIGH);
//        delay(500);
//        digitalWrite(13, LOW);
//        delay(1500);
//      }
//    }
//  };
};

RNimpl rn52;

void bt_setup(){
  pinMode(BT_GPIO9, OUTPUT);
  digitalWrite(BT_GPIO9, HIGH);
  pinMode(BT_GPIO2, INPUT);
  digitalWrite(BT_GPIO2, LOW);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  rn52.onGPIO2();
  bt_visible();
}

uint32_t lastHi2Lo = 0;
uint32_t triggerIn = 0;
int gpio2 = 0;
void bt_loop() {

  while (Serial.available()) {
    char c = Serial.read();
//    Serial.print(c);
    rn52.fromUART(c);
  }

  bool stateChanged = false;
  int n = digitalRead(BT_GPIO2);
  if (n != gpio2) {
    uint32_t now = millis();
    gpio2 = n;
    if (n) {
      // pin went hi
      if (now <= (lastHi2Lo+100)) {
        // last hi to lo transision is less than 150ms, this is a state change
        
        stateChanged = true;
        triggerIn = now + 100;
        if(!triggerIn)triggerIn=1;
      }
    } else {
      // pin went lo
      lastHi2Lo = now;
    }
  }
  if (triggerIn) {
    uint32_t now = millis();
    if(now >= triggerIn){
      triggerIn = 0;
      rn52.onGPIO2();
    }
  }
}

void RNimpl::toUART(const char* c, int len){
  for(int i=0;i<len;i++)
    Serial.write(c[i]);
};

void RNimpl::fromSPP(const char* c, int len){
  for (int i=0;i<len;i++)
    bt_sppRx(c[i]);
};

void RNimpl::setMode(Mode mode){
  if (mode == COMMAND) {
    digitalWrite(13, HIGH);
    digitalWrite(BT_GPIO9, LOW);
  } else if (mode == DATA) {
    digitalWrite(13, LOW);
    digitalWrite(BT_GPIO9, HIGH);
  }
};

const char *CMD_QUERY = "Q\r";
void RNimpl::onGPIO2() {
  queueCommand(CMD_QUERY);
}

void RNimpl::onProfileChange(BtProfile profile, bool connected) {
  switch(profile) {
    case A2DP:bt_a2dp = connected;
      if (connected && playing)bt_play();
      break;
    case SPP:bt_spp = connected;break;
    case IAP:bt_iap = connected;break;
    case HFP:bt_hfp = connected;break;
  }
}

void RNimpl::moduleSetup() {
  for(int i=0;i<SEQ_SETUP_LEN;i++)
    queueCommand(SEQ_SETUP[i]);
}

void bt_play() {
  rn52.sendAVCRP(RN52::RN52driver::PLAY);
}

void bt_pause() {
  rn52.sendAVCRP(RN52::RN52driver::PAUSE);
}

void bt_prev() {
  rn52.sendAVCRP(RN52::RN52driver::PREV);
}

void bt_next() {
  rn52.sendAVCRP(RN52::RN52driver::NEXT);
}

void bt_visible() {
  rn52.visible(true);
}

void bt_invisible() {
  rn52.visible(false);
}

void bt_reconnect() {
  rn52.reconnectLast();
}

void bt_disconnect() {
  rn52.disconnect();
}

void bt_moduleSetup() {
  rn52.moduleSetup();
}

void bt_sppTx(const uint8_t b) {
  rn52.toSPP(b);
}

#endif

