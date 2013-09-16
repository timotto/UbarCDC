#ifdef SPP_ANDROID

#define RX_IDLE          0x00
#define MSG_PING         0x10
#define MSG_PONG         0x11
#define MSG_DISCSELECT   0x20
#define MSG_TRACKINFO    0x80

int rxState = RX_IDLE;
int subState = 0;
int subPos = 0;

void cdc_select(int disc) {
  bt_sppTx(MSG_DISCSELECT);
  bt_sppTx((uint8_t)disc);
}

#define TRACK_INFO_MAX 20
int trackInfoPos = 0;
char* trackInfoString;
char trackInfoTitle[20];
char trackInfoArtist[20];
char trackInfoAlbum[20];

void bt_sppRx(const char c){
  switch(rxState) {
    case RX_IDLE:
      switch(c) {
        case MSG_TRACKINFO:
          rxState = MSG_TRACKINFO;
          subState = 0;
          subPos = 0;
          trackInfoString = trackInfoTitle;
          trackInfoPos = 0;
          break;
        case MSG_PING:
          rxState = MSG_PING;
          subState = 0;
          subPos = 0;
          break;
      }
      break;
    
    case MSG_TRACKINFO:
      if (c == 0) {
        if (trackInfoPos < TRACK_INFO_MAX) {
          trackInfoString[trackInfoPos++] = 0;
        }
        trackInfoPos=0;
        switch(subState) {
          case 0:
            trackInfoString = trackInfoArtist;
            subState=1;
            break;
          case 1:
            trackInfoString = trackInfoAlbum;
            subState=2;
            break;
          case 2:
            cdc_displayTrackinfo(trackInfoTitle, trackInfoArtist, trackInfoAlbum);
            rxState = RX_IDLE;
            break;
        }
      } else {
        if (trackInfoPos < TRACK_INFO_MAX) {
          trackInfoString[trackInfoPos++] = c;
        }
      }
      break;
      
    case MSG_PING:
      switch(subState) {
        case 0:
          bt_sppTx(MSG_PONG);
          bt_sppTx(c);
          rxState = RX_IDLE;
          break;
      }
      subState++;
      break;
  }
}

#endif

