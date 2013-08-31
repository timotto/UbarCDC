#ifndef CONSTSTRINGS_H
#define CONSTSTRINGS_H

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

#endif // CONSTSTRINGS_H
