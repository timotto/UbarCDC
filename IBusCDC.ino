#ifdef CDC_IBUS

#define DEBUG_CDC Serial.print

// SoftwareSerial
#define CDC_BUS_TX  12
#define CDC_BUS_RX  11

#include <EEPROM.h>
#include <IBUS_Protocol.h>
#include <SoftwareSerial.h>

//Ignition status, Off:
//80 04 BF 11 00 2A
//
//Ignition status, Pos1_Acc
//80 04 BF 11 01 2B
//
#define BTN_BMBT_LEFT    0x4810
#define BTN_BMBT_RIGHT   0x4800
#define BTN_MFL_UP       0x3B01
#define BTN_MFL_DOWN     0x3B08
#define BTN_PRESS        0
#define BTN_HOLD         1
#define BTN_RELEASE      2
#define DD_IG_OFF        0x1100
#define DD_IG_ACC        0x1101

void _ibus_tx(uint8_t c);
void _ibus_handleFrame();

SoftwareSerial ibus_serial(CDC_BUS_RX, CDC_BUS_TX, false, 'E');
IBUS_Protocol infotainmentBus(_ibus_tx);
IBUS_Frame *rxFrame;
bool inAux = false;
bool ignition = false;

void cdc_setup() {
  ibus_serial.begin(9600);
  pinMode(13, OUTPUT);
}

void cdc_loop() {
  ibus_serial.listen();
  while(ibus_serial.available()) {
    uint8_t c = ibus_serial.read();
    infotainmentBus.feed(c);
    if ( (rxFrame = infotainmentBus.getFrame()) != NULL) {
      blinkLED();
      _ibus_handleFrame();
    }
  }
}


void cdc_onResume() {
  // Kl.15 on
  int addr = 0;
  // magic
  if (
    EEPROM.read(addr++) == 'U' && 
    EEPROM.read(addr++) == 'b' && 
    EEPROM.read(addr++) == 'r' && 
    EEPROM.read(addr++) == 'C') {
    inAux = EEPROM.read(addr++);
  }
  cdc_setPlaying(inAux);
  cdc_setActive(inAux);
  
  onResume();
}

void cdc_onPause() {
  // Kl.15 off
  // TODO: save inAux state
  if (inAux) {
    cdc_setPlaying(false);
    cdc_setActive(false);
  }
  int addr = 0;
  // magic
  EEPROM.write(addr++, 'U');
  EEPROM.write(addr++, 'b');
  EEPROM.write(addr++, 'r');
  EEPROM.write(addr++, 'C');
  // data
  EEPROM.write(addr++, inAux);
  
  onPause();
}

const char inAuxData[] = {0x23, 0x62, 0x10, 'A', 'U', 'X'};
void _ibus_handleFrame() {

  DEBUG_CDC("IBus Frame from [");
  DEBUG_CDC(rxFrame->from(), HEX);
  DEBUG_CDC("] to [");
  DEBUG_CDC(rxFrame->to(), HEX);
  DEBUG_CDC("]\n");

  uint8_t *frameData = rxFrame->data();
  switch(rxFrame->to()){
    case 0x3b: // GT
//      DEBUG_CDC("to GT\n");
      switch(frameData[0]) {
        case 0x23: // display text
//          DEBUG_CDC("display text\n");
          if (rxFrame->len() >= 5) { // ll >= 7!
            bool match = true;
            for(int i=0;i<6;i++)
              if (frameData[i] != inAuxData[i]) {
//                DEBUG_CDC("mismatch on i=");
//                DEBUG_CDC(i, DEC);
//                DEBUG_CDC(": ");
//                DEBUG_CDC(frameData[i], DEC);
//                DEBUG_CDC(" vs ");
//                DEBUG_CDC(inAuxData[i], DEC);
//                DEBUG_CDC("\n");
                match = false;
                break;
              }
            if (match != inAux) {
              DEBUG_CDC(match?"ENABLE (in AUX)":"DISABLE (not in AUX)");
              cdc_setActive(match);
              cdc_setPlaying(match);
              inAux = match;
            }
          }
          return;
      }
      break;
  }
  
  switch(rxFrame->from()) {
    case 0x80: //IKE
      DEBUG_CDC("from IKE\n");
      switch(rxFrame->to()) {
        case 0xBF: //GLO
          DEBUG_CDC("to GLO, len=");
          DEBUG_CDC(rxFrame->len());
          DEBUG_CDC("\n");
          if (rxFrame->len() == 2) { //ll = 04!
            uint16_t code = (uint16_t)rxFrame->data()[0] << 8 | rxFrame->data()[1];
            DEBUG_CDC("Code: ");
            DEBUG_CDC(code);
            DEBUG_CDC("\n");
            switch(code) {
              case DD_IG_OFF:
                if (ignition) {
                  ignition=false;
                  cdc_onPause();
                }
                return;
              case DD_IG_ACC:
                if(!ignition) {
                  ignition = true;
                  cdc_onResume();
                }
                return;
            }
          }
          break;
      }
      break;
    case 0xF0: //BMBT
      switch(frameData[0]) {
        case 0x48: // Button
          _ibus_handleButton(0x4800 | frameData[1]);
          return;
      }
      break;
    case 0x50: //MFL
      switch(frameData[0]) {
        case 0x3B: // Button
          _ibus_handleButton(0x3B00 | frameData[1]);
          return;
      }
      break;
  }
}

void _ibus_handleButton(uint16_t btn) {
//  if(!inAux)return;
  int code = 0, state = 0;
  switch(btn & 0xff00) {
    case 0x4800: // BMBT buttons
      state = (btn & 0b11000000) >> 6;
      code = btn & 0b1111111100111111;
      break;
    case 0x3B00: // MFL buttons
      state = (btn & 0xf0) >> 4;
      code = btn & 0xff0f;
      break;
  }
  
  Serial.print("BTN, code:");
  Serial.print(code, HEX);
  Serial.print(", state: ");
  Serial.println(state, HEX);

  if (state == BTN_RELEASE) {
    switch(code) {
      case BTN_BMBT_LEFT:
      case BTN_MFL_DOWN:
        bt_prev();
        break;
      case BTN_BMBT_RIGHT:
      case BTN_MFL_UP:
        bt_next();
        break;
    }
  }
}

void _ibus_tx(uint8_t c) {
  ibus_serial.write(c);
}

static int blinkState = 0;
static void blinkLED() {
  blinkState = 1 - blinkState;
  digitalWrite(13, blinkState);
}

#endif

