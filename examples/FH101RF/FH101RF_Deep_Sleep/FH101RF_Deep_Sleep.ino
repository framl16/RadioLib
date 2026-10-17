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
#include <driver/rtc_io.h>

#define FH101RF_ID 0x1989
#define FH101RF_SPEED_LDR 7
#define FH101RF_SPEED_HDR 5

#define FH101RF_PIN_CS 15
#define FH101RF_PIN_IRQ 4
#define FH101RF_PIN_RST 0
#define FH101RF_PIN_GPIO 36

SPIClass spiFH(HSPI);
FH101RF fh101rf = new Module(FH101RF_PIN_CS, FH101RF_PIN_IRQ, FH101RF_PIN_RST, FH101RF_PIN_GPIO, spiFH);

bool did_wake_up() {
  esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
  return wakeup_cause == ESP_SLEEP_WAKEUP_EXT0;
}

void init_fh101rf(bool shouldReset) {
  Serial.print("[FH101RF] Initializing ... ");
  int16_t state = fh101rf.begin(shouldReset);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }
}

void configure_fh101rf() {
  Serial.print("[FH101RF] Configuring ...");
  
  fh101rf.setActiveBands(false, true, false);
  fh101rf.setActiveBranches(true, true, true);
  fh101rf.setReceiverId(FH101RF_ID);
  fh101rf.setIdMatchMode(RADIOLIB_FH101RF_ID_MATCH_INDIVIDUAL_ONLY);
  fh101rf.setIrqMode(RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH | RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_LDR | RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_FIFO);
  fh101rf.setSampleRate(RADIOLIB_FH101RF_SAMPLE_RATE_256, RADIOLIB_FH101RF_SAMPLE_RATE_1024);
  fh101rf.setFifoLength(RADIOLIB_FH101RF_FIFO_LENGTH_32_BITS);
}

void setup() {
  Serial.begin(9600);
  spiFH.begin();

  bool wakeup = did_wake_up();
  init_fh101rf(!wakeup);

  // the chip did wake up by a trigger from the FH101RF
  if(wakeup) {
    // handle IRQ
    Serial.println("Wakeup by FH101RF!");

    uint8_t irq = fh101rf.getIrqStatus();
    fh101rf.resetIrqStatus(irq);

    delay(5000);
  }

  // the board booted normally, so configure the FH101RF
  // to make it able to receive the designated wakeup signal
  else {
    configure_fh101rf();
  }

  Serial.println("Preparing wakeup configuration...");

  // configure the IRQ pin to wakeup the chip
  gpio_num_t wakeup_pin = (gpio_num_t) FH101RF_PIN_IRQ;
  esp_sleep_enable_ext0_wakeup(wakeup_pin, 1);
  rtc_gpio_pullup_dis(wakeup_pin);
  rtc_gpio_pulldown_en(wakeup_pin);

  // hold the RST pin high so that the FH101RF does not get reset during wakeup
  gpio_num_t rst_pin = (gpio_num_t) FH101RF_PIN_RST;
  gpio_hold_en(rst_pin);
  gpio_deep_sleep_hold_en();

  Serial.println("Going into deep sleep soon...");
  delay(200);
  esp_deep_sleep_start();

  return;
}

void loop() {
  // do nothing in case this gets executed somehow
  delay(1000);
}
