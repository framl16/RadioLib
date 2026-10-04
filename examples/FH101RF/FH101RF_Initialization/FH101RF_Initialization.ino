/*
  RadioLib FH101RF Initialization Example

  This example tries to initialize the FH101RF radio
  module and checks if it succeded with that.

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>

// FH101RF has the following connections:
// CS pin:   xxx
// IRQ pin:  xxx
// RESET pin:  xxx
// GPIO pin:  xxx
FH101RF radio = new Module(15, 22, 23, 36);

void setup() {
  Serial.begin(9600);
  SPI.begin(14, 12, 13, 15);

  // initialize FH101RF
  Serial.print(F("[FH101RF] Initializing ... "));
  int16_t state = radio.begin();

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("success!");
  } else {
    Serial.printf("failed. Code: %d\n", state);
    while (true) { delay(1000); }
  }
}

void loop() {
  // do nothing
  delay(1000);
}