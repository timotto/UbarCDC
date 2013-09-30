#ifdef SPP_ANDROID

#define RX_IDLE          0x00
#define MSG_PING         0x10
#define MSG_PONG         0x11
#define MSG_DISCSELECT   0x20
#define MSG_GETDIRECTORY 0x30
#define MSG_TRACKINFO    0x80
#define MSG_DIRECTORY    0x90

uint8_t rxState = RX_IDLE;
uint8_t subState = 0;
int subPos = 0;

uint8_t TRACK_INFO_MAX = 20;
int trackInfoPos = 0;
uint8_t* trackInfoString;

uint32_t currentIndexId = 0;
uint32_t backIndexId = 0;
uint16_t indexOffset = 0;
uint16_t indexLength = 0;
uint16_t currentIndexEntry = 0;


void spp_debug(int what) {
//  cdc_requestEntry(0, 0, 8);
}

void cdc_select(int disc) {
  bt_sppTx(MSG_DISCSELECT);
  bt_sppTx((uint8_t)disc);
}

/**
 * request a directory listing or select an entry
 * 
 * indexId is the id of the directory. The top level menu is id 0
 * offset of the currently displayed index, because low RAM and paging
 * length is how many entries max shall be returned
 **/
void cdc_requestEntry(uint32_t indexId, uint16_t offset, uint16_t length) {
  bt_sppTx(MSG_GETDIRECTORY);
  bt_sppTx(indexId >> 24);
  bt_sppTx(indexId >> 16);
  bt_sppTx(indexId >> 8);
  bt_sppTx(indexId & 0xff);
  bt_sppTx(offset >> 8);
  bt_sppTx(offset & 0xff);
  bt_sppTx(length >> 8);
  bt_sppTx(length & 0xff);
}

void bt_sppRx(const char c){
  switch(rxState) {
    case RX_IDLE:
      switch(c) {
        case MSG_TRACKINFO:
          rxState = MSG_TRACKINFO;
          subState = 0;
          subPos = 0;
          cdc_fetchTrackinfoString(&trackInfoString, 0, &TRACK_INFO_MAX);
          trackInfoPos = 0;
          break;
        case MSG_PING:
          rxState = MSG_PING;
          subState = 0;
          subPos = 0;
          break;
        case MSG_DIRECTORY:
          rxState = MSG_DIRECTORY;
          subState = 0;
          subPos = 0;
          currentIndexId = 0;
          backIndexId = 0;
          indexOffset = 0;
          indexLength = 0;
          currentIndexEntry = 0;
          break;
      }
      break;
    
    case MSG_TRACKINFO:
      if (c == 0) {
        cdc_saveTrackinfoString(subState, trackInfoPos);
        trackInfoPos=0;
        switch(subState) {
          case 0:
          case 1:
            subState=subState+1;
            cdc_fetchTrackinfoString(&trackInfoString, subState, &TRACK_INFO_MAX);
            break;
          case 2:
            cdc_displayTrackinfoStrings();
            rxState = RX_IDLE;
            break;
        }
      } else {
        if (trackInfoPos < TRACK_INFO_MAX) {
          trackInfoString[trackInfoPos++] = c;
        }
      }
      break;
      
    case MSG_DIRECTORY:
      switch(subState) {
        case 0:
          currentIndexId |= ((uint32_t)c) << (8 * (3 - subPos));
          
          if (subPos++ == 3){
            subPos=0;
            subState++;
          }
          break;
        case 1:
          backIndexId |= ((uint32_t)c) << (8 * (3 - subPos));
          
          if (subPos++ == 3){
            subPos=0;
            subState++;
          }
          break;
        case 2:
          indexOffset |= ((uint16_t)c) << (8 * (1 - subPos));
          
          if (subPos++ == 1){
            subPos=0;
            subState++;
          }
          break;
        case 3:
          indexLength |= ((uint16_t)c) << (8 * (1 - subPos));
          
          if (subPos++ == 1){
            subPos=0;
            trackInfoPos=0;
            subState++;
            cdc_fetchDirectoryString(&trackInfoString, currentIndexEntry, &TRACK_INFO_MAX);
          }
          break;
        case 4:
          if (c == 0) {
            cdc_saveDirectoryString(currentIndexEntry, trackInfoPos);
            if (currentIndexEntry >= indexLength) {
              // this must be the totally terminating 0 byte
              // trigger a show-on-display event
              cdc_displayDirectoryStrings();
              rxState = RX_IDLE;
            } else {
              // move target char* array to next entry pointer
              currentIndexEntry++;
              cdc_fetchDirectoryString(&trackInfoString, currentIndexEntry, &TRACK_INFO_MAX);
              trackInfoPos = 0;
            }
          } else {
            if (trackInfoPos < TRACK_INFO_MAX) {
              trackInfoString[trackInfoPos++] = c;
            }
          }
          break;
      }
      break;

    case MSG_PING:
      switch(subState) {
        case 0:
          bt_sppTx(MSG_PONG);
          bt_sppTx(c);
          rxState = RX_IDLE;
          
          spp_debug(1);
          
          break;
      }
      subState++;
      break;
  }
}

#endif

