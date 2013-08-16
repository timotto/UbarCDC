#include <AsyncMBus.h>
#include <TimerOne.h>

#define CDC_BUS_TX  9
#define CDC_BUS_RX  3

AsyncMBus mBus(CDC_BUS_RX, CDC_BUS_TX, onMbusMessage);

void setup() {
  Serial.begin(57600);
  mBus.setup();
  Serial.println("[main] started\n");
}

void loop() {
  mBus.loop();
}

void onMbusMessage(uint64_t msg, int len) {
  Serial.print("[MSG] ");
  if (msg > 0xffffffffull) {
    Serial.print((uint32_t)(msg >> 32), HEX);
  }
  Serial.println((uint32_t)(msg), HEX);
}

