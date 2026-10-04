#include "FH101RF.h"
#if !defined(RADIOLIB_EXCLUDE_FH101RF)

FH101RF::FH101RF(Module* mod) {
  _mod = mod;
}

int16_t FH101RF::begin() {
  // set module properties
  _mod->spiConfig.cmds[RADIOLIB_MODULE_SPI_COMMAND_READ] = RADIOLIB_FH101RF_CMD_READ;
  _mod->spiConfig.cmds[RADIOLIB_MODULE_SPI_COMMAND_WRITE] = RADIOLIB_FH101RF_CMD_WRITE;
  _mod->init();
  _mod->hal->pinMode(_mod->getIrq(), _mod->hal->GpioModeInput);
  _mod->hal->pinMode(_mod->getRst(), _mod->hal->GpioModeOutput);
  
  reset();

  // try to find the FH101RF chip
  uint8_t i = 0;
  bool flagFound = false;
  while ((i < 10) && !flagFound) {
    int16_t version = getChipVersion();
    if ((version == RADIOLIB_FH101RF_VERSION_CURRENT)) {
      flagFound = true;
    } else {
      RADIOLIB_DEBUG_BASIC_PRINTLN("FH101RF not found! (%d of 10 tries) RADIOLIB_FH101RF_REG_VERSION == 0x%04X, expected 0x0041", i + 1, version);
      _mod->hal->delay(10);
      i++;
    }
  }

  if (!flagFound) {
    RADIOLIB_DEBUG_BASIC_PRINTLN("No FH101RF found!");
    _mod->term();

    return(RADIOLIB_ERR_CHIP_NOT_FOUND);
  } else {
    RADIOLIB_DEBUG_BASIC_PRINTLN("M\tFH101RF");
  }

  // wait for the clock source to be stable
  i = 0;
  flagFound = false;
  while ((i < 10) && !flagFound) {
    // XTAL_GOOD typically transitions from 0 to 1 approximately 0.5s after the oscillation has started (room temperature)
    _mod->hal->delay(100);

    int16_t clockStable = isClockSourceStable();
    if (clockStable) {
      flagFound = true;
    } else {
      RADIOLIB_DEBUG_BASIC_PRINTLN("Clock source is not stable! (%d of 10 tries) RADIOLIB_FH101RF_REG_XTAL_GOOD == %d, expected 1", i + 1, clockStable);
      i++;
    }
  }

  return(RADIOLIB_ERR_NONE);
}

void FH101RF::reset() {
  _mod->hal->pinMode(_mod->getRst(), _mod->hal->GpioModeOutput);
  _mod->hal->digitalWrite(_mod->getRst(), _mod->hal->GpioLevelLow);
  _mod->hal->delay(1);
  _mod->hal->digitalWrite(_mod->getRst(), _mod->hal->GpioLevelHigh);
  _mod->hal->delay(500);
}

int16_t FH101RF::getChipVersion() {
  return(_mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_VERSION));
}

int16_t FH101RF::isClockSourceStable() {
  return(_mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_XTAL_GOOD));
}

#endif
