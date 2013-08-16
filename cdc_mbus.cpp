#include "settings.h"

#ifdef CDC_MBUS

#include "chms601.h"
#include "cdc.h"
#include <AsyncMBus.h>

#define RADIO_ADDRESS  9
#define CDC_ADDRESS    1

static void cdc_onMessage(uint64_t msg, int len);
static void cdc_onTx(uint64_t msg, bool success);
static void cdc_onFault(AsyncMBus::Fault fault, int arg);

static AsyncMBus mBus(CDC_BUS_RX, CDC_BUS_TX, cdc_onMessage, cdc_onTx, cdc_onFault);

typedef enum {
  OFF,
  INIT,
  IDLE,
  START,
  PLAYRESUME,
  PLAY,
  ACTIVE,
  PING_ACTIVE,
  PING_IDLE,
  CHANGE_DISC,
  STOP,
} State;
static void _cdc_updateChanged();
static void _cdc_beginState(State _state, int _sub = 0);
static void handleSequence(uint32_t now, t_cdc_sequence *sequence, State ret);
static void makePlayingTrack(uint64_t *m);
static void makeChangedCD(uint64_t *m);
static void makeCDStatus(uint64_t *m);

State state = INIT;
int substate = 0;
uint32_t nextStep = 0;
uint64_t playState;

bool active = false;

void cdc_active(bool a) {
  active = a;
}

void cdc_reset() {
  state = INIT;
}

void cdc_setup() {
  mBus.setup();
}

void cdc_loop() {
  mBus.loop();
  if(!active)return;
  uint32_t now = millis();
  switch(state) {
    case OFF:
      // TODO powersaving
      break;
      
    case INIT:
      if(!substate)_cdc_updateChanged();
      handleSequence(now, &SEQ_INIT, IDLE);
      break;
      
    case START:
      handleSequence(now, &SEQ_START, IDLE);
      break;
      
    case STOP:
      if (now < nextStep)break;
      switch(substate) {
        case 0:
          nextStep = now + 50;
          substate++;
          break;
        
        case 1:
          mBus.tx(0x9F00546);
          _cdc_beginState(IDLE);
          break;
      }

      break;
      
    case IDLE:
      break;
      
    case ACTIVE:
      if (now < nextStep)break;
      switch(substate) {
        case 0:
          nextStep = now + 50;
          substate = 1;
          break;
          
        case 1:
          nextStep = now + 1000;
          makePlayingTrack(&playState);
          mBus.tx(playState);
          break;
      }
      break;
      
    case PLAYRESUME:
      handleSequence(now, &SEQ_ONRESUME, PLAY);
      break;
      
    case PLAY:
      handleSequence(now, &SEQ_ONPLAY, ACTIVE);
      break;
      
    case CHANGE_DISC:
      if(!substate)_cdc_updateChanged();
      handleSequence(now, &SEQ_CHANGED, ACTIVE);
      break;
    
    case PING_ACTIVE:
      handleSequence(now, &SEQ_PING, ACTIVE);
      break;
      
    case PING_IDLE:
      handleSequence(now, &SEQ_PING, IDLE);
      break;
  }
}

static void handleSequence(uint32_t now, t_cdc_sequence *sequence, State ret) {
  if (now < nextStep)
    return;
  
//  Serial.print("[SEQ] ");Serial.println(substate);
  if (substate == 0) {
    // start of sequence. delay response by 50ms
    nextStep = now + 50;
  } else {
    if(!mBus.tx(sequence->messages[substate-1]))
      return; // retry next time
      
    nextStep = now + sequence->delay;
    
    if (substate >= sequence->length) {
      _cdc_beginState(ret);
      return;
    }
  }
  substate++;
}

void makePlayingTrack(uint64_t *m)
{
  *m=0x994000100000001ull;
  *m|=(uint64_t)(cdc_status.currentTrack%10)<<(10*4);
  *m|=(uint64_t)cdc_status.currentTrack<<(11*4);
  
  *m|=(uint64_t)(cdc_status.trackPosition%10)<<(4*4);
  *m|=(uint64_t)((cdc_status.trackPosition%100)/10)<<(5*4);
  *m|=(uint64_t)((cdc_status.trackPosition/60)%10)<<(6*4);
  *m|=(uint64_t)(((cdc_status.trackPosition/60)%100)/10)<<(7*4);
  
}

void makeChangedCD(uint64_t *m)
{
  *m=0x9B900000001ull;
  *m|=(uint64_t)cdc_status.currentDisc<<(7*4);
  *m|=(uint64_t)(cdc_status.trackPosition%10)<<(5*4);
  *m|=(uint64_t)(cdc_status.trackPosition/10)<<(6*4);
}

void makeCDStatus(uint64_t *m)
{
  *m=0x9C001999999Full;
  *m|=(uint64_t)cdc_status.currentDisc<<(9*4);
}

static inline uint8_t nibble(const uint64_t *m, int i, int l) {
  return (*m >> (l - (4 * (1+i))))&0xf;
}

static void _cdc_beginState(State _state, int _sub) {
  state = _state;
  substate = _sub;
  nextStep = 0;
}

static void _cdc_updateChanged() {
  makeChangedCD(&(_SEQ_CHANGED[0]));
  makeCDStatus(&(_SEQ_CHANGED[1]));
  makePlayingTrack(&(_SEQ_CHANGED[2]));
}

static void cdc_onMessage(uint64_t msg, int len) {
  Serial.print("[MSG] ");
  if (msg > 0xffffffffull) {
    Serial.print((uint32_t)(msg >> 32), HEX);
  }
  Serial.println((uint32_t)(msg), HEX);
  if(!active)return;
  uint8_t to = nibble(&msg, 0, len);
  if (CDC_ADDRESS != to)return; // ignore, not for me
  
  switch(msg) {
    case 0x19: // Startup
      _SEQ_START[2] = 0x9C0019900000ull;
      _SEQ_START[2] |= (uint64_t)cdc_status.currentDisc << (9 * 4);

      _SEQ_START[5] = 0x9B91000000Aull;
      _SEQ_START[5] |= (uint64_t)cdc_status.currentDisc << (6 * 4);
      _cdc_beginState(START);
      break;
    
    case 0x18: // Ping
      if (state == ACTIVE)
        _cdc_beginState(PING_ACTIVE);
      else
        _cdc_beginState(PING_IDLE);
      break;
      
    default:
      if ((msg & 0xfff00) == 0x11100) {
        // Set Play State
        switch (msg & 0xFF) {
          case 0x81:
            // play, resume
            _SEQ_ONRESUME[1] &= 0x9C0019900000ull;
            _SEQ_ONRESUME[1] |= (uint64_t)cdc_status.currentDisc << (9 * 4);
            _cdc_beginState(PLAYRESUME);
            break;
            
          case 0x01:
            cdc_status.selected = true;
            cdc_status.playing = true;
            if (state != ACTIVE && state != PLAY) {
              _SEQ_ONPLAY[3] &= 0x9C0019900000ull;
              _SEQ_ONPLAY[3] |= (uint64_t)cdc_status.currentDisc << (9 * 4);
              makeCDStatus(&(_SEQ_ONPLAY[7]));
              
              _SEQ_ONPLAY[9] = 0x99500011ull;
              _SEQ_ONPLAY[9]|=(uint64_t)(cdc_status.currentTrack%10)<<(3*4);
              _SEQ_ONPLAY[9]|=(uint64_t)cdc_status.currentTrack<<(4*4);
              
              _SEQ_ONPLAY[11] = 0x99400011ull;
              _SEQ_ONPLAY[11]|=(uint64_t)(cdc_status.currentTrack%10)<<(3*4);
              _SEQ_ONPLAY[11]|=(uint64_t)cdc_status.currentTrack<<(4*4);
              
              _cdc_beginState(PLAY);
            }
            cdc_onPlay();
            break;
          case 0x02:
            Serial.println("[PAUSE]");
            // pause
            cdc_status.playing = false;
            cdc_onPause();
            break;
          case 0x04:
            // ffwd
            break;
          case 0x08:
            // frwd
            break;
          case 0x10:
            // scan stop
            break;
          case 0x20:
            // NA
            break;
          case 0x40:
            Serial.println("[STOP]");
            // stop
            cdc_status.playing = false;
            cdc_status.selected = false;
            _cdc_beginState(STOP);
            cdc_selected(false);
            break;
        }
      } else 
      // select disc / track
      if ((msg & 0xfff00000) == 0x11300000) {
        const int disc = (msg >> 16) & 0xf;
        const int track = (msg >> 8) & 0xff;
        // play pause random flags in lowest 8 bits
        
        if (disc != 0) {
          cdc_status.currentDisc = disc;
          cdc_status.currentTrack = 1;
          cdc_status.trackPosition = 0;
          _cdc_beginState(CHANGE_DISC);
          cdc_onSelectCd(disc);
        } else {
          cdc_status.currentTrack = track;
          cdc_status.trackPosition = 0;
          _cdc_beginState(CHANGE_DISC);
          cdc_onSelectTrack(track);
        }
      }
      break;
  }
}

static void cdc_onTx(uint64_t msg, bool success) {
}

static void cdc_onFault(AsyncMBus::Fault fault, int arg) {
  Serial.print("[FAULT] ");Serial.print(fault);Serial.print(" ");Serial.println(arg);
}

#define dec2bcd(d) ((d / 10) << 4 | (d%10))

uint8_t bcd2dec(uint8_t b) {
  return (b >> 4) * 10 | (b & 0xf);
}


#endif

