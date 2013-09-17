#ifdef CDC_IBUS

// http://github.com/timotto/IBUS_Protocol
#include <IBUS_Protocol.h>
// http://pjrc.com/
#include <AltSoftSerial.h>
#include <EEPROM.h>

//#define DEBUG_CDC(x) Serial.print(x)
#define DEBUG_CDC

//Ignition status, Off:
//80 04 BF 11 00 2A
//
//Ignition status, Pos1_Acc
//80 04 BF 11 01 2B
//
// Display off
// 46 05 FF D5 01 01 69
// Display on
// 46 05 FF D5 01 00 68
// 

// Wait for IBUS_TX_WAIT milliseconds after receiving the last frame before sending
#define IBUS_TX_WAIT     50

#define IBUS_GT          0x3b
#define IBUS_CID         0x46
#define IBUS_MFL         0x50
#define IBUS_IKE         0x80
#define IBUS_GLO         0xBF
#define IBUS_BMBT        0xF0
#define IBUS_LOC         0xFF

#define BTN_BMBT_LEFT    0x4810
#define BTN_BMBT_RIGHT   0x4800
#define BTN_BMBT_CD1     0x4811
#define BTN_BMBT_CD2     0x4801
#define BTN_BMBT_CD3     0x4812
#define BTN_BMBT_CD4     0x4802
#define BTN_BMBT_CD5     0x4813
#define BTN_BMBT_CD6     0x4803
#define BTN_MFL_UP       0x3B01
#define BTN_MFL_DOWN     0x3B08
#define BTN_PRESS        0
#define BTN_HOLD         1
#define BTN_RELEASE      2
#define DD_IG_OFF        0x1100
#define DD_IG_ACC        0x1101

void _ibus_tx(uint8_t c);
void _ibus_handleFrame();

AltSoftSerial ibus_serial;
IBUS_Protocol infotainmentBus(_ibus_tx);
IBUS_Frame *rxFrame;
bool inAux = true;
bool ignition = true;
bool displayOpen = true;

void cdc_setup() {
  ibus_serial.begin(9600, SERIAL_8E1);
}

uint32_t lastRx = 0;
void cdc_loop() {
  uint32_t now = millis();
  while(ibus_serial.available()) {
    lastRx = now;
    infotainmentBus.feed(ibus_serial.read());
    if ( (rxFrame = infotainmentBus.getFrame()) != NULL) {
      _ibus_handleFrame();
    }
  }
  if (lastRx + 50 >= now) {
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
    displayOpen = EEPROM.read(addr++);
  }
  cdc_setPlaying(inAux);
  cdc_setActive(inAux);
  
  onResume();
}

void cdc_onPause() {
  // Kl.15 off
  if (inAux) {
    cdc_setPlaying(false);
    cdc_setActive(false);
  }
  onPause();
}

char* lastTitle = 0;
char* lastArtist = 0;
char* lastAlbum = 0;

void cdc_displayTrackinfo(char* title, char* artist, char* album) {
  lastTitle = title;
  lastArtist = artist;
  lastAlbum = album;
  _cdc_sendTrackinfo();
}

void _cdc_sendTrackinfo() {
  if(!displayOpen || !inAux || !(lastTitle && lastArtist && lastAlbum))return;

  // 2012-08-25 19:41:37.968:  68 11 3B A5 63 01 41 4D 6F 6F 6E 62 6F 6F 74 69 63 61 9A
  // 2012-08-25 19:41:37.968:  ! RAD  --> GT  : Screen Text, Layout=0x63:  FC2=0x01  Fld1,PartTx="Moonbootica"
  uint8_t ll = strlen(lastTitle) + 6;
  uint8_t ck = 0;
  _ibus_tx(0x68); ck ^= 0x68;
  _ibus_tx(ll); ck ^= ll;
  _ibus_tx(0x3b); ck ^= 0x3b;
  _ibus_tx(0xa5); ck ^= 0xa5;
  _ibus_tx(0x63); ck ^= 0x63;
  _ibus_tx(0x01); ck ^= 0x01;
  _ibus_tx(0x41); ck ^= 0x41;
  for(int i=0;i<ll-6;i++) {
    _ibus_tx(lastTitle[i]);
    ck ^= lastTitle[i];
  }
  _ibus_tx(ck);
  
  // 2012-08-25 19:41:37.996:  68 17 3B A5 63 01 42 34 34 31 30 30 20 28 69 6E 74 65 72 6C 75 64 65 29 AD
  // 2012-08-25 19:41:37.996:  ! RAD  --> GT  : Screen Text, Layout=0x63:  FC2=0x01  Fld2,PartTx="44100 (interlude)"
  ll = strlen(lastArtist) + 6;
  ck = 0;
  _ibus_tx(0x68); ck ^= 0x68;
  _ibus_tx(ll); ck ^= ll;
  _ibus_tx(0x3b); ck ^= 0x3b;
  _ibus_tx(0xa5); ck ^= 0xa5;
  _ibus_tx(0x63); ck ^= 0x63;
  _ibus_tx(0x01); ck ^= 0x01;
  _ibus_tx(0x42); ck ^= 0x42;
  for(int i=0;i<ll-6;i++) {
    _ibus_tx(lastArtist[i]);
    ck ^= lastArtist[i];
  }
  _ibus_tx(ck);
  
  // 2012-08-25 19:41:38.021:  68 17 3B A5 63 01 43 4D 6F 6F 6E 6C 69 67 68 74 20 57 65 6C 66 61 72 65 F3
  // 2012-08-25 19:41:38.021:  ! RAD  --> GT  : Screen Text, Layout=0x63:  FC2=0x01  Fld3,PartTx="Moonlight Welfare"
  ll = strlen(lastAlbum) + 6;
  ck = 0;
  _ibus_tx(0x68); ck ^= 0x68;
  _ibus_tx(ll); ck ^= ll;
  _ibus_tx(0x3b); ck ^= 0x3b;
  _ibus_tx(0xa5); ck ^= 0xa5;
  _ibus_tx(0x63); ck ^= 0x63;
  _ibus_tx(0x01); ck ^= 0x01;
  _ibus_tx(0x43); ck ^= 0x43;
  for(int i=0;i<ll-6;i++) {
    _ibus_tx(lastAlbum[i]);
    ck ^= lastAlbum[i];
  }
  _ibus_tx(ck);
  
  // 2012-08-25 19:41:38.048:  68 06 3B A5 63 00 00 93
  // 2012-08-25 19:41:38.048:  ! RAD  --> GT  : Screen Text, Layout=0x63:  FC2=0x00  Fld0,EndTx=""
  _ibus_tx(0x68);
  _ibus_tx(0x06);
  _ibus_tx(0x3b);
  _ibus_tx(0xa5);
  _ibus_tx(0x63);
  _ibus_tx(0x00);
  _ibus_tx(0x00);
  _ibus_tx(0x93);
}

const char inAuxData[] = {0x23, 0x62, 0x10, 'A', 'U', 'X'};
void _ibus_handleFrame() {
  
  uint8_t from = rxFrame->from();
  uint8_t to = rxFrame->to();
  uint8_t dlen = rxFrame->len();

//  DEBUG_CDC("IBus Frame from [");
//  DEBUG_CDC(from, HEX);
//  DEBUG_CDC("] to [");
//  DEBUG_CDC(to, HEX);
//  DEBUG_CDC("]\n");

  uint8_t *frameData = rxFrame->data();
  
  if (from == IBUS_CID && to == IBUS_LOC && dlen == 3 && frameData[0] == 0xD5 && frameData[1] == 0x01) {
    displayOpen = frameData[2] == 0x00;
    _ibus_persistState();
    return;
  }
  
  switch(to){
    case IBUS_GT: // GT
//      DEBUG_CDC("to GT\n");
      switch(frameData[0]) {
        case 0x23: // display text
//          DEBUG_CDC("display text\n");
          if (dlen >= 5) { // ll >= 7!
            bool match = true;
            for(int i=0;i<6;i++)
              if (frameData[i] != inAuxData[i]) {
                match = false;
                break;
              }
            if (match != inAux) {
              DEBUG_CDC(match?"ENABLE (in AUX)":"DISABLE (not in AUX)");
              cdc_setActive(match);
              cdc_setPlaying(match);
              
              if (inAux)
                _cdc_sendTrackinfo();
                
              inAux = match;
              _ibus_persistState();
            }
          }
          return;
      }
      break;
  }
  
  switch(from) {
    case IBUS_IKE: //IKE
      DEBUG_CDC("from IKE\n");
      switch(to) {
        case IBUS_GLO: //GLO
//          DEBUG_CDC("to GLO, len=");
//          DEBUG_CDC(rxFrame->len());
//          DEBUG_CDC("\n");
          if (dlen == 2 && frameData[0] == 0x11) { //ll = 04!
//            uint16_t code = (uint16_t)rxFrame->data()[0] << 8 | rxFrame->data()[1];
//            DEBUG_CDC("Code: ");
//            DEBUG_CDC(code);
//            DEBUG_CDC("\n");
            switch(frameData[1]) {
              case 0x00:
                if (ignition) {
                  ignition=false;
                  cdc_onPause();
                }
                return;
              default:
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
    case IBUS_BMBT:
      switch(frameData[0]) {
        case 0x48: // Button
          _ibus_handleButton(0x4800 | frameData[1]);
          return;
      }
      break;
    case IBUS_MFL: //MFL
      switch(frameData[0]) {
        case 0x3B: // Button
          _ibus_handleButton(0x3B00 | frameData[1]);
          return;
      }
      break;
  }
}

int prev_state = 0;
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
  
  DEBUG_CDC("BTN, code:");
  DEBUG_CDC(code, HEX);
  DEBUG_CDC(", state: ");
  DEBUG_CDC(state, HEX);

  if (state == BTN_RELEASE) {
    if (prev_state == BTN_PRESS) {
      switch(code) {
        case BTN_BMBT_LEFT:
        case BTN_MFL_DOWN:
          bt_prev();
          break;
        case BTN_BMBT_RIGHT:
        case BTN_MFL_UP:
          bt_next();
          break;
        case BTN_BMBT_CD1:
          cdc_select(1);
          break;
        case BTN_BMBT_CD2:
          cdc_select(2);
          break;
        case BTN_BMBT_CD3:
          cdc_select(3);
          break;
        case BTN_BMBT_CD4:
          cdc_select(4);
          break;
        case BTN_BMBT_CD5:
          cdc_select(5);
          break;
        case BTN_BMBT_CD6:
          cdc_select(6);
          break;
      }
    } else {
      // ffwd / rwd
    }
  } else prev_state = state;
}

void _ibus_tx(uint8_t c) {
  ibus_serial.write(c);
}

void _ibus_persistState() {
  int addr = 0;
  // magic
  EEPROM.write(addr++, 'U');
  EEPROM.write(addr++, 'b');
  EEPROM.write(addr++, 'r');
  EEPROM.write(addr++, 'C');
  // data
  EEPROM.write(addr++, inAux);
  EEPROM.write(addr++, displayOpen);
}

void bus_tx(uint8_t c) {_ibus_tx(c);}
#endif

