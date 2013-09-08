#ifndef CONSTSTRINGS_H
#define CONSTSTRINGS_H


#ifdef BT_OVC3860
const char *BT_O_CMD_PREFIX = "AT#";
const char *BT_O_CMD_POSTFIX = "\r\n";
const char *BT_O_CMD_RECONNECT = "MI";
const char *BT_O_CMD_DISCONNECT = "MJ";
const char *BT_O_CMD_QAVCRP = "MO";
const char *BT_O_CMD_FFWDON = "MR";
const char *BT_O_CMD_RWDON = "MS";
const char *BT_O_CMD_FFRWDOFF = "MT";
const char *BT_O_CMD_QA2DP = "MV";
const char *BT_O_CMD_PLAYPAUSE = "MA";
const char *BT_O_CMD_STOP = "MC";
const char *BT_O_CMD_NEXT = "MD";
const char *BT_O_CMD_PREV = "ME";
const char *BT_O_CMD_PAIR_ON = "CA";
const char *BT_O_CMD_PAIR_OFF = "CB";
const char *BT_O_CMD_OFF = "VX";

const char *BT_O_STATE_CONNECTED1 = "MB";
const char *BT_O_STATE_CONNECTED2 = "MA";
const char *BT_O_STATE_DISCONNECTED = "MY";
const char *BT_O_STATE_PAUSE = "MP";
const char *BT_O_STATE_PLAYING = "MR";
const char *BT_O_STATE_SPP_ON = "SC";
const char *BT_O_STATE_SPP_OFF = "SD";
const char *BT_O_STATE_PAIRING_ON = "II";
const char *BT_O_STATE_PAIRING_OFF = "IJ2";

#endif

#ifdef BT_RN52
//const char *SEQ_SETUP[] = {"S|,02", "S-,Jaguar XJS-C", "SD,04", "SK,04", "SP,31337", "R,1"};
const char *SEQ_SETUP[] = {"S|,02", "S-,BMW Z4", "SD,04", "SK,04", "SP,31337", "R,1"};
int SEQ_SETUP_LEN = 6;
const char *BT_CMD_PLAYPAUSE = "AP";
const char *BT_CMD_PREV = "AT-";
const char *BT_CMD_NEXT = "AT+";
const char *BT_CMD_VISIBLE = "@,1";
const char *BT_CMD_INVISIBLE = "@,0";
const char *BT_CMD_RECONNECT = "B";
const char *BT_CMD_DISCONNECT = "K,04";
#endif

#endif // CONSTSTRINGS_H
