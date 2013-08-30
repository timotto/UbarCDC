#ifdef CDC_IBUS

#define DEBUG_CDC Serial.print
// SoftwareSerial
#define CDC_BUS_TX  12
#define CDC_BUS_RX  11

#include <IBUS_Protocol.h>
#include <SoftwareSerial.h>

void _ibus_tx(uint8_t c);
void _ibus_handleFrame();

SoftwareSerial ibus_serial(CDC_BUS_RX, CDC_BUS_TX, false, 'E');
IBUS_Protocol infotainmentBus(_ibus_tx);
IBUS_Frame *rxFrame;

void cdc_setup() {
  ibus_serial.begin(9600);
  pinMode(13, OUTPUT);
}

void cdc_loop() {
  while(ibus_serial.available()) {
    uint8_t c = ibus_serial.read();
    infotainmentBus.feed(c);
    if ( (rxFrame = infotainmentBus.getFrame()) != NULL) {
      _ibus_handleFrame();
    }
  }
}

void _ibus_handleFrame() {

  DEBUG_CDC("IBus Frame from [");
  DEBUG_CDC(rxFrame->from(), HEX);
  DEBUG_CDC("] to [");
  DEBUG_CDC(rxFrame->to(), HEX);
  DEBUG_CDC("]\n");

  switch(rxFrame->from()) {
    case 0xF0: //BMBT
      switch(rxFrame->data()[0]) {
        case 0x48: // Button
          _ibus_handleButton(0x4800 | rxFrame->data()[1]);
          break;
      }
      break;
    case 0x50: //MFL
      switch(rxFrame->data()[0]) {
        case 0x3B: // Button
          _ibus_handleButton(0x3B00 | rxFrame->data()[1]);
          break;
      }
      break;
  }
}

void _ibus_handleButton(uint16_t btn) {
  int code = 0, state = 0;
  switch(btn & 0xff00) {
    case 0x4800: // BMBT buttons
      state = (btn & (0b11 << 6)) >> 6;
      code = (btn & 0x00111111);
      break;
    case 0x3B00: // MFL buttons
      state = btn & 0x0f;
      code = btn & 0xf0;
      break;
  }
  Serial.print("BTN, code:");
  Serial.print(code, HEX);
  Serial.print(", state: ");
  Serial.println(state, HEX);
}

void _ibus_tx(uint8_t c) {
  ibus_serial.write(c);
}

#endif

