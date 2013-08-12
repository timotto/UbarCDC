#include "settings.h"

#ifdef CDC_MBUS
#include "cdc.h"
#include <MBus.h>

static MBus mBus(CDC_BUS_RX, CDC_BUS_TX);
static bool isOn = false;

void cdc_setup() {
  cdc_status.playing = false;
  cdc_status.currentDisc = 1;
  cdc_status.currentTrack = 1;
  cdc_status.trackPosition = 1;
}

void cdc_loop() {
  uint64_t cdcRxMsg=0;
  if(mBus.receive(&cdcRxMsg)) {
    DEBUG("[CDC] received message: ");
    DEBUG((uint16_t)(cdcRxMsg >> 16), HEX);
    DEBUG((uint16_t)(cdcRxMsg & 0xffff), HEX);
    DEBUG("\n");
  }
}

void cdc_loopREAL() {
  uint64_t cdcRxMsg=0;
  if(mBus.receive(&cdcRxMsg)) {
    
    switch(cdcRxMsg)  {
      case Ping:
        mBus.send(PingOK);
        break;
        
      case 0x19:
        mBus.sendChangedCD(cdc_status.currentDisc,cdc_status.currentTrack);
        delay(50);
        mBus.sendCDStatus(cdc_status.currentDisc);
        delay(50);
        mBus.sendPlayingTrack(cdc_status.currentTrack,0);
        break;
        
      case OFF:
        if (isOn) {
          mBus.send(Wait);
          isOn=false;
          cdc_selected(false);
        }
        break;
      
      case Play:
        mBus.sendPlayingTrack(cdc_status.currentTrack,cdc_status.trackPosition);
        break;
      
    }
    
  }
}

#endif

