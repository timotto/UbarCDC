#ifdef SPP_JSON

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

void bt_sppRx(const char c){
  switch(rxState) {
    case RX_IDLE:
      switch(c) {
        case MSG_TRACKINFO:
          rxState = MSG_TRACKINFO;
          subState = 0;
          subPos = 0;
          break;
        case MSG_PING:
          rxState = MSG_PING;
          subState = 0;
          subPos = 0;
          break;
      }
      break;
    
    case MSG_TRACKINFO:
      switch(subState) {
        case 0:
          break;
        default:
          rxState = RX_IDLE;
          break;
      }
      subState++;
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

