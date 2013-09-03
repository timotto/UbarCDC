#define DEBUG_BT Serial.print

// https://github.com/rabinath/Readline
#include <Readline.h>
#include <SoftwareSerial.h>
#include "ConstStrings.h"

SoftwareSerial bt_serial(BT_RX, BT_TX); // RX, TX

bool bt_sendCommand(const char *cmd, bool enter = true, bool exit = true);
bool bt_enterCmdMode();
bool bt_exitCmdMode();

ReadLine bt_readLine;
uint32_t lastHi2Lo = 0;
int gpio2 = 0;
uint8_t lastState[2] = {0};
int cmdModeDepth = 0;

// bool set by bt_play/pause commands, also used in Q response in bt_loop()
bool bt_shallPlay = false;

void bt_setup() {
  pinMode(BT_GPIO9, OUTPUT);
  digitalWrite(BT_GPIO9, HIGH);
  pinMode(BT_GPIO2, INPUT);
  digitalWrite(BT_GPIO2, LOW);
  pinMode(PWR_BT, OUTPUT);
  digitalWrite(PWR_BT, LOW);
  bt_serial.begin(9600);
}

void bt_on() {
  digitalWrite(PWR_BT, HIGH);
}

void bt_off() {
  digitalWrite(PWR_BT, LOW);
}

void bt_loop() {
  bool stateChanged = false;
  int n = digitalRead(BT_GPIO2);
  if (n != gpio2) {
    uint32_t now = millis();
    gpio2 = n;
    DEBUG_BT(now);
    DEBUG_BT(" GPIO2: ");DEBUG_BT(gpio2);DEBUG_BT("\n");
    if (n) {
      // pin went hi
      if (now <= (lastHi2Lo+100)) {
        // last hi to lo transision is less than 150ms, this is a state change
        
        stateChanged = true;
        _bt_queryState(now);
        
      }
    } else {
      // pin went lo
      lastHi2Lo = now;
    }
  }
}

void _bt_queryState(uint32_t now) {
  if(!bt_sendCommand("Q", true, false))
    return;
  
  int lines = 1;
  do {
    char *line;
    if ((line=bt_readLine.feed(&bt_serial))) {
      bt_iap = line[1] & 0x01;
      bt_spp = line[1] & 0x02;
      bt_a2dp = line[1] & 0x04;
      bt_hfp = line[1] & 0x08;
      
      bt_state = line[3];
      if (bt_state >= '0' && bt_state <= '9') {
        bt_state = bt_state - '0';
      } else if (bt_state >= 'a' && bt_state <= 'f') {
        bt_state = bt_state - 'a' + 10;
      } else if (bt_state >= 'A' && bt_state <= 'F') {
        bt_state = bt_state - 'A' + 10;
      } else bt_state = 0;
      
      DEBUG_BT("BT> ");
      DEBUG_BT(line);
      DEBUG_BT("\n");
      bt_dumpState();
      
      if (line[1] != lastState[0] || bt_state != lastState[1]) {
        lastState[0] = line[1];
        lastState[1] = bt_state;
        bt_stateChanged();
      }
      
      if (bt_a2dp && bt_shallPlay && bt_state != 'D')
        bt_play();
      
      if (--lines == 0)
        break;
    }
    if (now + 1000 < millis()) {
      Serial.println("Timeout requesting status");
      break;
    }
  } while(1);

  bt_exitCmdMode();
}

bool _bt_assertA2dpStateCmd(uint8_t state, const char *cmd) {
  if (bt_a2dp) {
    if (bt_state == state) {
      bt_sendCommand(cmd);
      return true;
    } else {
      DEBUG_BT("[BT] bt_state no match, want [");
      DEBUG_BT(state, DEC);
      DEBUG_BT("] have [");
      DEBUG_BT(bt_state, DEC);
      DEBUG_BT("]\n");
    }
  } else {
    DEBUG_BT("[BT] A2DP not connected\n");
  }
  return false;
}

void bt_play() {
  bt_shallPlay = true;
  _bt_assertA2dpStateCmd(0x3, BT_CMD_PLAYPAUSE);
}

void bt_pause() {
  bt_shallPlay = false;
  _bt_assertA2dpStateCmd(0xd, BT_CMD_PLAYPAUSE);
}

void bt_prev() {
  bt_shallPlay = true;
  if(!_bt_assertA2dpStateCmd(0xd, BT_CMD_PREV))
    _bt_assertA2dpStateCmd(0x3, BT_CMD_PREV);
}

void bt_next() {
  bt_shallPlay = true;
  if(!_bt_assertA2dpStateCmd(0xd, BT_CMD_NEXT))
    _bt_assertA2dpStateCmd(0x3, BT_CMD_NEXT);
}

void bt_visible() {
  bt_sendCommand(BT_CMD_VISIBLE);
}

void bt_invisible() {
  bt_sendCommand(BT_CMD_INVISIBLE);
}

void bt_reconnect() {
  bt_sendCommand(BT_CMD_RECONNECT);
}

void bt_disconnect() {
  bt_sendCommand(BT_CMD_DISCONNECT);
}

void bt_moduleSetup() {
  bt_sendCommands(100, SEQ_SETUP_LEN, SEQ_SETUP);
}

bool bt_sendCommands(int d, int argc, const char **argv) {
  if (argc<1)
    return true;
  
  if(!bt_sendCommand(argv[0], true, false)) {
    bt_exitCmdMode();
    return false;
  }

  if (argc<2) {
    bt_exitCmdMode();
    return true;
  }
  if(d)delay(d);
  for(int i=1;i<argc-1;i++) {
    if(!bt_sendCommand(argv[i], false, false)) {
      bt_exitCmdMode();
      return false;
    }
    if(d)delay(d);
  }
  
  if(!bt_sendCommand(argv[argc-1], false, true)) {
    bt_exitCmdMode();
    return false;
  }
  return true;
}

bool bt_sendCommand(const char *cmd, bool enter, bool exit) {
  if(enter && !bt_enterCmdMode())
    return false;
  
  bt_serial.print(cmd);
  bt_serial.write('\r');
//  Serial.print("<BT ");
//  Serial.println(cmd);
  
  if (exit && !bt_exitCmdMode())
    return false;
    
  return true;
}

bool bt_enterCmdMode() {
  if (cmdModeDepth) {
    DEBUG_BT("Already in command mode...\n");
    cmdModeDepth++;
    return true;
  }

  DEBUG_BT("Entering command mode...\n");
  bt_serial.listen();
  uint32_t now = millis();
  digitalWrite(BT_GPIO9, LOW);
  do {
    char *line;
    if ((line=bt_readLine.feed(&bt_serial))) {
      if (0 == strcmp("CMD", line))
        break;
      else {
        DEBUG_BT("unexpected line from BT:\n");
        DEBUG_BT(line);
        DEBUG_BT("\n");
      }
    }
    
    if (now + 1000 < millis()) {
      DEBUG_BT("Timeout entering CMD mode\n");
      digitalWrite(BT_GPIO9, HIGH);
      return false;
    }
  } while(1);
  cmdModeDepth++;
  return true;
};

bool bt_exitCmdMode(){
  cmdModeDepth--;
  if (cmdModeDepth) {
    DEBUG_BT("Not leaving command mode, nested...\n");
    return true;
  }
  DEBUG_BT("Leaving command mode...\n");
  uint32_t now = millis();
  digitalWrite(BT_GPIO9, HIGH);
  do {
    char *line;
    if ((line=bt_readLine.feed(&bt_serial))) {
      if (0 == strcmp("END", line))
        break;
      else {
        DEBUG_BT("unexpected line from BT:\n");
        DEBUG_BT(line);
        DEBUG_BT("\n");
      }
    }
    
    if ((now + 1000) < millis()) {
      DEBUG_BT("Timeout leaving CMD mode\n");
      return false;
    }
  } while(1);
  return true;
};

void bt_dumpState() {
  Serial.print("BT>");
  Serial.print(" State: ");
  Serial.print(bt_state);
  if(bt_iap)Serial.print(" iAP");
  if(bt_spp)Serial.print(" SPP");
  if(bt_a2dp)Serial.print(" A2DP");
  if(bt_hfp)Serial.print(" HFP");
  Serial.println();
}


