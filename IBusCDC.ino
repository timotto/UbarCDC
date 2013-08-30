#ifdef CDC_IBUS

#define DEBUG_CDC Serial.print
// SoftwareSerial
#define CDC_BUS_TX  12
#define CDC_BUS_RX  11

#include <IBUS_Protocol.h>
#include <SoftwareSerial.h>

void _ibus_tx(uint8_t c);
void _ibus_handleFrame();

SoftwareSerial ibus_serial(CDC_BUS_RX, CDC_BUS_TX);
IBUS_Protocol infotainmentBus(_ibus_tx);
IBUS_Frame *rxFrame;

void cdc_setup() {
  ibus_serial.begin(9600);
  pinMode(13, OUTPUT);
}

void cdc_loop() {
  while(ibus_serial.available()) {
    uint8_t c = ibus_serial.read();
    DEBUG_CDC("I[");
    DEBUG_CDC(c, HEX);
    DEBUG_CDC("] ");
    infotainmentBus.feed(c);
    if ( (rxFrame = infotainmentBus.getFrame()) != NULL) {
      _ibus_handleFrame();
    }
  }
}

int ledState = 0;
void _ibus_handleFrame() {
  ledState = 1 - ledState;
  digitalWrite(13, ledState);
  DEBUG_CDC("IBus Frame from [");
  DEBUG_CDC(rxFrame->from(), HEX);
  DEBUG_CDC("] to [");
  DEBUG_CDC(rxFrame->to(), HEX);
  DEBUG_CDC("]\n");
}

void _ibus_tx(uint8_t c) {
  ibus_serial.write(c);
}

#endif

