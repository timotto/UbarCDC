int currentTrack = 0;

// called from mBus.loop() method
void onMbusMessage(uint64_t msg, int len) {
  Serial.print("[MSG] ");
  if (msg > 0xffffffffull) {
    Serial.print((uint32_t)(msg >> 32), HEX);
  }
  Serial.println((uint32_t)(msg), HEX);
  
  // enter or exit active mode based on the selected CD, example: 9C0001575020
  if((msg & 0xFF0000000000ull) == 0x9C0000000000ull) {
    // (from CDC) disc status
    bool isSpecialDisc = ((msg & 0x00000FFFFFF0ull) == 0x000001575020ull); // 15 tracks, 75 minutes, 0 seconds

    if (isSpecialDisc != active)
      cdc_setActive(isSpecialDisc);

    return;
  }
  
  // don't do anything els if not active
  if (!active)return;
  
  if ((msg & 0xFFF00) == 0x11100) {
    // (from radio) set play state
    uint8_t state = (msg & 0x77);
    switch(state) {
      case 0x01:
        cdc_setPlaying(true);
        break;
      case 0x02:
      case 0x40:
        cdc_setPlaying(false);
        break;
    }
    return;
  }
  
  // play state
  // 994150110001
  //    --  -----
  //    |   || |
  //    1   23 4
  // 1: track, 2:
  if (len >= 48) {
    uint64_t mask = 0xFFFull << (len - 12);
    uint64_t playState = 0x994ull << (len - 12);
    if ((msg & mask) == playState) {
      currentTrack = ((msg >> (len-16))&0xf) * 10;
      currentTrack |= ((msg >> (len-20))&0xf);
      
      Serial.print("[CDC] currentTrack=");Serial.println(currentTrack);
      return;
    }
  }
  
  if ((msg & 0xFFF00000) == 0x11300000) {
    int nextTrack = ((msg >> 8) & 0xf) * 10;
    nextTrack |= ((msg >> 12) & 0x0f);
    
    if (nextTrack > currentTrack)
      bt_next();
    else if (nextTrack < currentTrack)
      bt_prev();

  }
}


