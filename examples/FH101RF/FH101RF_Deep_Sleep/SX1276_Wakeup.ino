/*
  RadioLib FH101RF Wakeup Example

  This example uses an SX1276 to wake the FH101RF.
  The FH101RF then generates an IRQ signal.
  Both chips are connected to a single ESP32 MCU.
  This example was tested on a LilyGO TTGO 2.1.6 board.

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h> 

#define FH101RF_ID 0x1989
#define FH101RF_SPEED_LDR 7
#define FH101RF_SPEED_HDR 5

#define SX1276_PIN_CS 18
#define SX1276_PIN_IRQ 26
#define SX1276_PIN_RST 23
#define SX1276_PIN_GPIO 33
#define SX1276_PIN_DIO2 32

SX1276 sx1276 = new Module(SX1276_PIN_CS, SX1276_PIN_IRQ, SX1276_PIN_RST, SX1276_PIN_GPIO);

void wakeup(uint16_t id, uint8_t* data, uint8_t length) {
  uint8_t payloadLength = RADIOLIB_FH101RF_ID_LENGTH + length;
  uint8_t payload[payloadLength];
  payload[0] = (id >> 8);
  payload[1] = (id & 0xFF);

  if(data != nullptr) {
    memcpy(payload + 2, data, length);
  }

  // calculate delays
  uint16_t delaySlow = FH101RF::getSymbolDuration(FH101RF_SPEED_LDR);
  uint16_t delayFast = FH101RF::getSymbolDuration(FH101RF_SPEED_HDR);

  // prepare preamble modulation
  uint16_t symbolsPreamble = RADIOLIB_FH101RF_SYMBOL_SIZE;
  uint8_t modulationPreamble[symbolsPreamble] = {0};
  FH101RF::getModulationForPreamble(RADIOLIB_FH101RF_CODE_MLS_A, modulationPreamble);

  // prepare payload modulation
  uint16_t symbolsPayload = RADIOLIB_FH101RF_SYMBOL_SIZE * RADIOLIB_FH101RF_SYMBOLS_PER_BYTE * payloadLength;
  uint8_t modulationPayload[symbolsPayload] = {0};
  FH101RF::getModulationForPayload(RADIOLIB_FH101RF_CODE_MLS_A, RADIOLIB_FH101RF_CODE_MLS_B, payload, modulationPayload, payloadLength);

  // activate direct mode transmitter
  Serial.println("Starting transmission.");
  int16_t state = sx1276.transmitDirect();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.println(F("[SX1276] Unable to start direct transmission mode, code "));
    Serial.println(state);
  }

  // send preamble symbols
  for(int i = 0; i < symbolsPreamble; i++) {
    digitalWrite(SX1276_PIN_DIO2, modulationPreamble[i]);
    delayMicroseconds(delaySlow);
  }

  digitalWrite(SX1276_PIN_DIO2, LOW);
  delay(5);

  // send payload symbols
  for(int i = 0; i < symbolsPayload; i++) {
    digitalWrite(SX1276_PIN_DIO2, modulationPayload[i]);
    delayMicroseconds(delayFast);
  }

  Serial.println("Ending transmission.");
  digitalWrite(SX1276_PIN_DIO2, LOW);
  sx1276.finishTransmit();
}

void setup() {
  Serial.begin(9600);

  // initialize SX1276
  Serial.print(F("[SX1276] Initializing ... "));

  int16_t state = sx1276.beginFSK(868.0, 1.024, 10, 125, 2, 16, true);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }

  pinMode(SX1276_PIN_DIO2, OUTPUT);
  digitalWrite(SX1276_PIN_DIO2, LOW);

  return;
}

void loop() {
  // send wakeup signal
  wakeup(FH101RF_ID, nullptr, 0);

  // wait a bit before transmitting again
  delay(30000);
}
