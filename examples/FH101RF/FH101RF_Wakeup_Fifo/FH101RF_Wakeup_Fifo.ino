/*
  RadioLib FH101RF Wakeup Example

  This example uses an SX1276 to wake the FH101RF.
  The FH101RF then generates an IRQ signal.
  Both chips are connected to a single ESP32 MCU.
  This example was tested on a LilyGO TTGO 2.1.6 board.
  
  The transmission is done via the fifo queue of the FSK modem.

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>
#include <SPI.h>

#define FH101RF_ID 0x1989
#define FH101RF_SPEED_LDR 5
#define FH101RF_SPEED_HDR 3
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

#define FIFO_CHUNK_SIZE 30

SPIClass spiFH(HSPI);
SPIClass spiSX(VSPI);

FH101RF fh101rf = new Module(FH101RF_PIN_CS, FH101RF_PIN_IRQ, FH101RF_PIN_RST, FH101RF_PIN_GPIO, spiFH);
SX1276 sx1276 = new Module(SX1276_PIN_CS, SX1276_PIN_IRQ, SX1276_PIN_RST, SX1276_PIN_GPIO, spiSX);

bool triggered = false;
bool transmitting = true;

uint8_t modulation[1024] = {0};
uint16_t modulationLength = 0;
uint16_t remainingLength = 0;

unsigned long lastWakeup = 0;

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

  modulationLength = FH101RF::getModulationLength(FH101RF_SPEED_LDR, FH101RF_SPEED_HDR, payloadLength);
  memset(modulation, 0, modulationLength);

  FH101RF::getModulation(FH101RF_SPEED_LDR, FH101RF_SPEED_HDR, RADIOLIB_FH101RF_CODE_MLS_A, RADIOLIB_FH101RF_CODE_MLS_B, payload, payloadLength, modulation);

  // start sending payload
  Serial.println("Starting transmission.");
  sx1276.fixedPacketLengthMode(0);
  sx1276.startTransmit(modulation, modulationLength);
  sx1276.setFifoThreshold(FIFO_CHUNK_SIZE / 2);

  remainingLength = modulationLength - FIFO_CHUNK_SIZE;
  transmitting = true;
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
  state = fh101rf.setIrqMode(RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_LDR);
  state = fh101rf.setSampleRate(RADIOLIB_FH101RF_SAMPLE_RATE_1024, RADIOLIB_FH101RF_SAMPLE_RATE_4096);
  state = fh101rf.setFifoLength(RADIOLIB_FH101RF_FIFO_LENGTH_32_BITS);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }
  
  // initialize SX1276
  Serial.print(F("[SX1276] Initializing ... "));

  state = sx1276.beginFSK(868.0, 4.096, 10, 125, 2, 16, true);
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
}

void loop() {
  if (transmitting) {
    if(remainingLength > 0) {
      if (!sx1276.isFifoLevel()) {
        uint8_t amount = min(remainingLength, (uint16_t) FIFO_CHUNK_SIZE);
        uint16_t pos = modulationLength - remainingLength;

        sx1276.getMod()->SPIwriteRegisterBurst(RADIOLIB_SX127X_REG_FIFO, modulation + pos, amount);
        remainingLength -= amount;
      }
    } else {
      if (sx1276.isFifoEmpty()) {
        delayMicroseconds(300);
        sx1276.standby();
        transmitting = false;
      }
    }
  }

  if (triggered) {
    // the LED should have turned on now
    delay(3000);
    
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
  }

  // if we waited long enough, transmit wakeup (again)
  if(millis() - lastWakeup > 10000) {
    uint8_t data[2] = { 0x47, 0x11 };
    wakeup(FH101RF_ID, data, sizeof(data));

    lastWakeup = millis();
  }
}
