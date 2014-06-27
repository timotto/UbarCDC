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
#define IBUS_RAD         0x68
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

const uint8_t inAuxDataA[] = {0x23, 0x62, 0x10, 0x41, 0x55, 0x58};
const uint8_t inAuxDataB[] = {0xA5, 0x62, 0x01, 0x00};

uint8_t ibus_msg_title[32] = {0x68, 0x00, 0x3b, 0xa5, 0x63, 0x01, 0x41};
uint8_t ibus_msg_artist[32] = {0x68, 0x00, 0x3b, 0xa5, 0x63, 0x01, 0x42};
uint8_t ibus_msg_album[32] = {0x68, 0x00, 0x3b, 0xa5, 0x63, 0x01, 0x43};
uint8_t ibus_msg_taa[] = {0x68, 0x06, 0x3b, 0xa5, 0x63, 0x00, 0x00, 0x93};

// C0 / 41: C0 = line0, set cursor (1100.0000), 41 = line1 (0100.0001)
uint8_t ibus_msg_index0[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0xC0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index1[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x41, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index2[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x42, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index3[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index4[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x44, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index5[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index6[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x46, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_index7[22] = {0x68, 0x14, 0x3B, 0x21, 0x60, 0x01, 0x47, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00};
uint8_t ibus_msg_i07[8]     = {0x68, 0x06, 0x3B, 0xA5, 0x60, 0x01, 0x00, 0x91};

IBUS_Frame trackInfoFrames[4];
IBUS_Frame menuContentFrames[9];
uint8_t inAuxState = 0;

void _ibus_tx(uint8_t c);
void _ibus_handleFrame();

AltSoftSerial ibus_serial;
IBUS_Protocol infotainmentBus(_ibus_tx);
IBUS_Frame *rxFrame;
bool inAux = true;
bool ignition = true;
bool displayOpen = true;
bool displayTrackInfoStrings = false;
bool displayMenuContent = false;

void cdc_setup() {
  ibus_serial.begin(9600, SERIAL_8E1);
  trackInfoFrames[0].setPacket(ibus_msg_title);
  trackInfoFrames[1].setPacket(ibus_msg_artist);
  trackInfoFrames[2].setPacket(ibus_msg_album);
  trackInfoFrames[3].setPacket(ibus_msg_taa);

  menuContentFrames[0].setPacket(ibus_msg_index0);
  menuContentFrames[1].setPacket(ibus_msg_index1);
  menuContentFrames[2].setPacket(ibus_msg_index2);
  menuContentFrames[3].setPacket(ibus_msg_index3);
  menuContentFrames[4].setPacket(ibus_msg_index4);
  menuContentFrames[5].setPacket(ibus_msg_index5);
  menuContentFrames[6].setPacket(ibus_msg_index6);
  menuContentFrames[7].setPacket(ibus_msg_index7);
  menuContentFrames[8].setPacket(ibus_msg_i07);
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
  if (now >= (lastRx + 50)) {
    if (displayMenuContent) {
      displayMenuContent = false;
      for(int i=0;i<9;i++)
        infotainmentBus.sendFrame(&(menuContentFrames[i]));
    }
    if (displayTrackInfoStrings) {
      displayTrackInfoStrings = false;
      for(int i=0;i<4;i++) {
        infotainmentBus.sendFrame(&(trackInfoFrames[i]));
      }
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

void cdc_fetchTrackinfoString(uint8_t **dst, uint8_t index, uint8_t *maxLength) {
  uint8_t* msg;
  switch(index) {
    case 0:
    case 1:
    case 2:
      *maxLength = 24;
      msg = _cdc_trackInfoString(index);
      msg[1] = 0;
      *dst = &(msg[7]);
      break;
    default:
      *dst = 0;
      *maxLength = 0;
      break;
  }
}

void cdc_saveTrackinfoString(uint8_t index, uint8_t length) {
  uint8_t* msg = _cdc_trackInfoString(index);
  if (msg == 0)return;
  msg[1] = length + 6;
  
  uint8_t ck = 0;
  int end = msg[1] + 1;
  for(int i=0;i<end;i++)
    ck ^= msg[i];
  msg[end] = ck;
}

void cdc_displayTrackinfoStrings() {
  if(!displayOpen || !inAux
     || ibus_msg_title[1] == 0 || ibus_msg_artist[1] == 0 || ibus_msg_album[1] == 0)return;
     
  displayTrackInfoStrings = true;
}

uint8_t* _cdc_trackInfoString(int index) {
  switch(index) {
    case 0:
      return ibus_msg_title;
    case 1:
      return ibus_msg_artist;
    case 2:
      return ibus_msg_album;
  }
  return 0;
}

void cdc_fetchDirectoryString(uint8_t **dst, uint8_t index, uint8_t *maxLength) {
  *maxLength = 14;
  uint8_t *msg = menuContentFrames[index].packet();
  *dst = &(msg[7]);
  memset(dst, 0x20, *maxLength);
}

void cdc_saveDirectoryString(uint8_t index, uint8_t length) {
  uint8_t *msg = menuContentFrames[index].packet();
  msg[1] = length + 6;
  
  uint8_t ck = 0;
  int end = msg[1] + 1;
  for(int i=0;i<end;i++)
    ck ^= msg[i];
  msg[end] = ck;
}

void cdc_displayDirectoryStrings() {
  if(!displayOpen || !inAux)return;
  displayMenuContent = true;
}

void _ibus_handleFrame() {
  
  uint8_t from = rxFrame->from();
  uint8_t to = rxFrame->to();
  uint8_t dlen = rxFrame->len();

  uint8_t *frameData = rxFrame->data();
  
  if (from == IBUS_CID && to == IBUS_LOC && dlen == 3 && frameData[0] == 0xD5 && frameData[1] == 0x01) {
    displayOpen = frameData[2] == 0x00;
    _ibus_persistState();
    return;
  }
  
  if (inAuxState == 1 && from == IBUS_GT && to == IBUS_RAD && frameData[0] == 0x22 && dlen == 3) {
    inAuxState = 0;
    if (frameData[2] == 0x06) {
      // 7 confirmed messages, most likely the blank AUX screen
      if (!inAux) {
        inAux = true;
        inAuxState = 0;
        cdc_setActive(true);
        cdc_setPlaying(true);
        
        _ibus_persistState();
      }
      cdc_displayTrackinfoStrings();
    } else {
      // some other screen
    }
  }
  
  if (from == IBUS_RAD && to == IBUS_GT && frameData[0] == 0x23) {
    bool outOfAux = false;
    if (dlen == 6) {
      bool match = true;
      for(int i=0;i<6;i++)
        if (frameData[i] != inAuxDataA[i]) {
          match = false;
          break;
        }
      if (match != inAux) {
        if (match) {
          inAuxState = 1;
        } else {
          outOfAux = true;
        }
      }
    } else {
      outOfAux = true;
    }
    
    if (outOfAux) {
      inAux = false;
      inAuxState = 0;
      cdc_setActive(false);
      cdc_setPlaying(false);
      
      _ibus_persistState();
    } else {
      cdc_displayTrackinfoStrings();
    }
    return;
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

