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
#include <SPI.h>

#define FH101RF_ID 0x1989
#define FH101RF_SPEED_LDR 7
#define FH101RF_SPEED_HDR 5
#define FH101RF_PIN_CS 15
#define FH101RF_PIN_IRQ 4
#define FH101RF_PIN_RST 0
#define FH101RF_PIN_GPIO 36

#define SX1276_PIN_CS 18
#define SX1276_PIN_IRQ 26
#define SX1276_PIN_RST 23
#define SX1276_PIN_GPIO 33
#define SX1276_PIN_DIO2 32

#define PIN_LED 21

SPIClass spiFH(HSPI);
SPIClass spiSX(VSPI);

FH101RF fh101rf = new Module(FH101RF_PIN_CS, FH101RF_PIN_IRQ, FH101RF_PIN_RST, FH101RF_PIN_GPIO, spiFH);
SX1276 sx1276 = new Module(SX1276_PIN_CS, SX1276_PIN_IRQ, SX1276_PIN_RST, SX1276_PIN_GPIO, spiSX);

bool triggered = false;
void IRAM_ATTR callback() {
  digitalWrite(PIN_LED, HIGH);
  triggered = true;
}

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

  spiFH.begin();
  spiSX.begin();

  // initialize FH101RF
  Serial.print("[FH101RF] Initializing ... ");
  int16_t state = fh101rf.begin();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }

  // configure FH101RF
  Serial.print("[FH101RF] Configuring ... ");
  
  state = fh101rf.setActiveBands(false, true, false);
  state = fh101rf.setActiveBranches(true, true, true);
  state = fh101rf.setReceiverId(FH101RF_ID);
  state = fh101rf.setIdMatchMode(RADIOLIB_FH101RF_ID_MATCH_INDIVIDUAL_ONLY);
  state = fh101rf.setIrqMode(RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH | RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_LDR | RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_FIFO);
  state = fh101rf.setSampleRate(RADIOLIB_FH101RF_SAMPLE_RATE_256, RADIOLIB_FH101RF_SAMPLE_RATE_1024);
  state = fh101rf.setFifoLength(RADIOLIB_FH101RF_FIFO_LENGTH_32_BITS);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }
  
  // initialize SX1276
  Serial.print(F("[SX1276] Initializing ... "));

  state = sx1276.beginFSK(868.0, 1.024, 10, 125, 2, 16, true);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }

  pinMode(PIN_LED, OUTPUT);
  pinMode(SX1276_PIN_DIO2, OUTPUT);

  digitalWrite(PIN_LED, LOW);
  digitalWrite(SX1276_PIN_DIO2, LOW);

  attachInterrupt(FH101RF_PIN_IRQ, callback, HIGH);
  delay(5000);
  return;
}

void loop() {
  // send wakeup signal
  uint8_t data[2] = { 0x47, 0x11 };
  wakeup(FH101RF_ID, data, sizeof(data));

  // the LED should have turned on now
  delay(3000);

  if (triggered) {
    // turn LED off
    digitalWrite(PIN_LED, LOW);

    Serial.println("LED should have been on until now.");

    // check irq of the FH101RF
    uint8_t irq = fh101rf.getIrqStatus();

    // some data was (maybe) put in the fifo buffer
    if(irq & RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_FIFO || irq & RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_LDR) {
      Serial.println("Some data was written into the fifo buffer.");

      uint8_t fifoBits = fh101rf.getFifoCount();
      uint8_t fifoBytes = fifoBits / 8;

      uint8_t buffer[fifoBytes] = { 0 };
      fh101rf.getFifoBuffer(buffer, fifoBytes);

      Serial.printf("Received a payload of %d bytes: ", fifoBytes);
      
      for(int i = 0; i < fifoBytes; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
      }

      Serial.println();
    }

    // only the ID was matched
    else if(irq & RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH) {
      Serial.println("Only the configured ID of the FH101RF was received.");
    }

    Serial.println("Resetting the FH101RF for the next run.");
    fh101rf.resetIrqStatus(irq);
    triggered = false;
  } else {
    Serial.println("The interrupt has not triggered, something went wrong.");
  }

  // wait a bit before transmitting again
  delay(10000);
}
