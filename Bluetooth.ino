#include <SoftwareSerial.h>

SoftwareSerial bt_serial(10, 11); // RX, TX

char *SEQ_SETUP[] = {"S|,02", "S-,Jaguar XJS-C", "SD,06", "SK,04", "SP,31337", "R,1"};
int SEQ_SETUP_LEN = 6;

bool bt_sendCommand(char *cmd, bool enter = true, bool exit = true);
bool bt_enterCmdMode();
bool bt_exitCmdMode();

ReadLine bt_readLine;
uint32_t lastHi2Lo = 0;
int gpio2 = 0;
uint8_t lastState[2] = {0};

// bool set by bt_play/pause commands, also used in Q response in bt_loop()
bool bt_shallPlay = false;

void _bt_assertA2dpStateCmd(uint8_t state, char *cmd) {
  if (bt_a2dp) {
    if (bt_state == state) {
      bt_sendCommand(cmd);
    } else {
      Serial.print("[BT] bt_state no match, want [");
      Serial.print(state, DEC);
      Serial.print("] have [");
      Serial.print(bt_state, DEC);
      Serial.println("]");
    }
  } else {
    Serial.println("[BT] A2DP not connected");
  }
}

void bt_play() {
  bt_shallPlay = true;
  _bt_assertA2dpStateCmd(0x3, "AP");
}

void bt_pause() {
  bt_shallPlay = false;
  _bt_assertA2dpStateCmd(0xd, "AP");
}

void bt_prev() {
  bt_shallPlay = true;
  _bt_assertA2dpStateCmd(0xd, "AT-");
}

void bt_next() {
  bt_shallPlay = true;
  _bt_assertA2dpStateCmd(0xd, "AT+");
}

void bt_visible() {
  bt_sendCommand("@,1");
}

void bt_invisible() {
  bt_sendCommand("@,0");
}

void bt_reconnect() {
  bt_sendCommand("B");
}

void bt_moduleSetup() {
  bt_sendCommands(100, SEQ_SETUP_LEN, SEQ_SETUP);
}

void bt_setup() {
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH);
  pinMode(7, INPUT);
  digitalWrite(7, LOW);
  bt_serial.begin(9600);
}

void bt_loop() {
  int n = digitalRead(7);
  if (n != gpio2) {
    uint32_t now = millis();
    gpio2 = n;
    Serial.print(now);
    Serial.print(" GPIO2: ");Serial.println(gpio2);
    if (n) {
      // pin went hi
      if (now <= (lastHi2Lo+100)) {
        // last hi to lo transision is less than 150ms, this is a state change
        
        
        if(!bt_sendCommand("Q", true, false))
          return;

        int lines = 1;
        do {
          char *line;
          if (line=bt_readLine.feed(&bt_serial)) {
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
            
            Serial.print("BT> ");
            Serial.println(line);
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
            return;
          }
        } while(1);

        bt_exitCmdMode();
      }
    } else {
      // pin went lo
      lastHi2Lo = now;
    }
  }
}

bool bt_sendCommands(int d, int argc, char **argv) {
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

bool bt_sendCommand(char *cmd, bool enter, bool exit) {
  if(enter && !bt_enterCmdMode())
    return false;
  
  bt_serial.print(cmd);
  bt_serial.write('\r');
  Serial.print("<BT ");
  Serial.println(cmd);
  
  if (exit && !bt_exitCmdMode())
    return false;
    
  return true;
}

// nada, gpio9 is lo by setup()
bool bt_enterCmdMode() {
  Serial.println("Entering command mode...");
  uint32_t now = millis();
  digitalWrite(6, LOW);
  do {
    char *line;
    if (line=bt_readLine.feed(&bt_serial)) {
      if (0 == strcmp("CMD", line))
        break;
      else {
        Serial.println("unexpected line from BT:");
        Serial.println(line);
      }
    }
    
    if (now + 1000 < millis()) {
      Serial.println("Timeout entering CMD mode");
      return false;
    }
  } while(1);
  return true;
};

bool bt_exitCmdMode(){
  Serial.println("Leaving command mode...");
  uint32_t now = millis();
  digitalWrite(6, HIGH);
  do {
    char *line;
    if (line=bt_readLine.feed(&bt_serial)) {
      if (0 == strcmp("END", line))
        break;
      else {
        Serial.println("unexpected line from BT:");
        Serial.println(line);
      }
    }
    
    if ((now + 1000) < millis()) {
      Serial.println("Timeout leaving CMD mode");
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


