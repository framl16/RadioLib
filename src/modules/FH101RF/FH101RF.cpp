#include "FH101RF.h"
#if !defined(RADIOLIB_EXCLUDE_FH101RF)

static const uint32_t correlatorSequences[RADIOLIB_FH101RF_AMOUNT_CORRELATION_PATTERNS] = {
  1791846223, // RADIOLIB_FH101RF_CODE_MLS_A
  1832425331, // RADIOLIB_FH101RF_CODE_MLS_B
  1913874966, // RADIOLIB_FH101RF_CODE_MLS_C
   233608647, // RADIOLIB_FH101RF_CODE_MLS_D
   355637424, // RADIOLIB_FH101RF_CODE_MLS_A_INV
   315058316, // RADIOLIB_FH101RF_CODE_MLS_B_INV
  1113190842, // RADIOLIB_FH101RF_CODE_M_SEQUENCE_A
  1126685782, // RADIOLIB_FH101RF_CODE_M_SEQUENCE_B
           0, // RADIOLIB_FH101RF_CODE_31_ZEROS
  2139095040, // RADIOLIB_FH101RF_CODE_8_ONES
  2147450880, // RADIOLIB_FH101RF_CODE_16_ONES
  2147483520, // RADIOLIB_FH101RF_CODE_24_ONES
  2147483647, // RADIOLIB_FH101RF_CODE_31_ONES
  1431655765, // RADIOLIB_FH101RF_CODE_0101_PATTERN
  1717986918, // RADIOLIB_FH101RF_CODE_1100_PATTERN
  1908874353  // RADIOLIB_FH101RF_CODE_111000_PATTERN
};

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

  // This is set according to 5.7 of the datasheet
  int16_t state = setDCornerCtrl(0x02);
  RADIOLIB_ASSERT(state);

  state = setLcTgEna(0x00);
  RADIOLIB_ASSERT(state);

  state = setComparatorThreshold(0x0A);
  RADIOLIB_ASSERT(state);

  // Calibration
  state = calibrate();
  RADIOLIB_ASSERT(state);

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

int16_t FH101RF::setDCornerCtrl(uint8_t value) {
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_D_CORNER_CTRL, value));
}

int16_t FH101RF::setLcTgEna(uint8_t value) {
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_LC_TG_ENA, value));
}

int16_t FH101RF::calibrate() {
  int16_t state = calibrateLocalOscillator();
  RADIOLIB_ASSERT(state);

  state = calibrateSamplePulse();
  RADIOLIB_ASSERT(state);

  state = calibrateComparator();
  RADIOLIB_ASSERT(state);

  return(RADIOLIB_ERR_NONE);
}

int16_t FH101RF::setActiveBands(bool band433, bool band868, bool band2G4) {
  int16_t current = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL);
  if (current < 0) {
    return(current);
  }

  uint8_t value = (uint8_t) current;
  if (band433) {
    value |= RADIOLIB_FH101RF_BAND_433_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BAND_433_MASK;
  }

  if (band868) {
    value |= RADIOLIB_FH101RF_BAND_868_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BAND_868_MASK;
  }

  if (band2G4) {
    value |= RADIOLIB_FH101RF_BAND_2G4_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BAND_2G4_MASK;
  }

  return(setBandBranchControlRaw(value));
}

int16_t FH101RF::setActiveBranches(bool branchWeak, bool branchMedium, bool branchStrong) {
  int16_t current = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL);
  if (current < 0) {
    return(current);
  }

  uint8_t value = (uint8_t) current;
  if (branchWeak) {
    value |= RADIOLIB_FH101RF_BRANCH_WEAK_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BRANCH_WEAK_MASK;
  }

  if (branchMedium) {
    value |= RADIOLIB_FH101RF_BRANCH_MEDIUM_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BRANCH_MEDIUM_MASK;
  }

  if (branchStrong) {
    value |= RADIOLIB_FH101RF_BRANCH_STRONG_MASK;
  } else {
    value &= ~RADIOLIB_FH101RF_BRANCH_STRONG_MASK;
  }

  return(setBandBranchControlRaw(value));
}

int16_t FH101RF::setSampleRate(uint8_t srPreamble, uint8_t srFastRx) {
  RADIOLIB_CHECK_RANGE(srPreamble, 0b000, 0b111, RADIOLIB_ERR_INVALID_SAMPLE_RATE);
  RADIOLIB_CHECK_RANGE(srFastRx, 0b000, 0b111, RADIOLIB_ERR_INVALID_SAMPLE_RATE);

  int16_t state = _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA433_SLOW, srPreamble);
  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA868_SLOW, srPreamble);
  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA2G4_SLOW, srPreamble);

  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA433_FAST, srFastRx);
  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA868_FAST, srFastRx);
  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_NFA2G4_FAST, srFastRx);

  return(state);
}

int16_t FH101RF::setIrqMode(uint8_t mode) {
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_IRQ_SELECT, mode));
}

int16_t FH101RF::getIrqStatus() {
  return(_mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_IRQ_STATUS));
}

int16_t FH101RF::resetIrqStatus(uint8_t mode) {
  // Apprarently, this register is not cleared automatically by the chip once the
  // reset has been applied. Because of this, the previous value is remembered
  // and written back into the register
  uint8_t prev = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_IRQ_CLR);

  int16_t state = _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_IRQ_CLR, mode);
  state |= _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_IRQ_CLR, prev);

  return(state);
}

int16_t FH101RF::setIdMatchMode(uint8_t mode) {
  RADIOLIB_CHECK_RANGE(mode, 0b00, 0b11, RADIOLIB_ERR_INVALID_ID_MATCH_MODE);
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_IDM_CTRL, mode));
}

int16_t FH101RF::setReceiverId(uint16_t id) {
  uint8_t high = id >> 8;
  uint8_t low = id & 0xFF;

  int16_t state = _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_ID_HI, high);
  RADIOLIB_ASSERT(state);

  state = _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_ID_LO, low);
  return(state);
}

int16_t FH101RF::setCorrelationPatterns(uint8_t codeA, uint8_t codeB) {
  RADIOLIB_CHECK_RANGE(codeA, 0x0, 0xF, RADIOLIB_ERR_INVALID_CORRELATION_PATTERN);
  RADIOLIB_CHECK_RANGE(codeB, 0x0, 0xF, RADIOLIB_ERR_INVALID_CORRELATION_PATTERN);

  uint8_t value = codeB << 4 | codeA;
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_CODE_SELECT, value));
}

int16_t FH101RF::setBandBranchControlRaw(uint8_t value) {
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL, value));
}

int16_t FH101RF::startCalibration(uint8_t value) {
  RADIOLIB_CHECK_RANGE(value, 0b10, 0b1000, RADIOLIB_ERR_INVALID_CALIBRATION_TYPE);

  uint8_t calibValue = value | RADIOLIB_FH101RF_CALIBRATION_ACTIVE;
  // TODO: "correct" SPIsetRegValue write function is too paranoid
  // This register gets cleared when the calibration is initialized by the chip
  // This process happens too quickly, so the write function fails
  _mod->SPIwriteRegister(RADIOLIB_FH101RF_REG_CALIB_CTRL, calibValue);

  uint8_t i = 0;
  bool flagFound = false;
  while ((i < 100) && !flagFound) {
    _mod->hal->delay(10);

    bool started = isCalibrationStarted();
    if (started) {
      flagFound = true;
    } else {
      RADIOLIB_DEBUG_BASIC_PRINTLN("Calibration has not started! (%d of 10 tries)", i + 1);
    }

    i++;
  }

  if(!flagFound) {
    return(RADIOLIB_ERR_CALIBRATION_TIMEOUT);
  }

  i = 0;
  flagFound = false;
  while ((i < 10) && !flagFound) {
    _mod->hal->delay(100);

    bool running = isCalibrationRunning();
    if (!running) {
      flagFound = true;
    } else {
      RADIOLIB_DEBUG_BASIC_PRINTLN("Calibration is still running! (%d of 10 tries)", i + 1);
    }

    i++;
  }

  if(!flagFound) {
    return(RADIOLIB_ERR_CALIBRATION_TIMEOUT);
  }

  return(RADIOLIB_ERR_NONE);
}

bool FH101RF::isCalibrationStarted() {
  int16_t value = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_CALIB_CTRL);
  return(!(value & RADIOLIB_FH101RF_CALIBRATION_ACTIVE));
}

bool FH101RF::isCalibrationRunning() {
  int16_t value = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_CALIB_STATUS);
  return(value & RADIOLIB_FH101RF_CALIBRATION_ACTIVE);
}

int16_t FH101RF::calibrateLocalOscillator() {
  return startCalibration(RADIOLIB_FH101RF_CALIBRATION_OSCILLATOR);
}

int16_t FH101RF::calibrateSamplePulse() {
  // According to 7.3.3 of the datasheet, this needs to be set to 0x46
  // for optimal current consumption
  int16_t state = _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_N_SPG_TARGET, 0x46);
  RADIOLIB_ASSERT(state);

  return startCalibration(RADIOLIB_FH101RF_CALIBRATION_SAMPLE_PULSE);
}

int16_t FH101RF::calibrateComparator() {
  // For calibration, set the chip to be active on all bands and branches
  // Keep the current value to set it back once the calibration has finished
  uint8_t cache = _mod->SPIgetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL);
  _mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL, 0b111, 6, 4);
  
  // According to 7.3.1 of the datasheet, this has to be set to 0x0A
  setComparatorThreshold(0x0A);

  int16_t state = startCalibration(RADIOLIB_FH101RF_CALIBRATION_COMPARATOR);
  RADIOLIB_ASSERT(state);

  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL, cache));
}

int16_t FH101RF::setComparatorThreshold(uint8_t value) {
  return(_mod->SPIsetRegValue(RADIOLIB_FH101RF_REG_COMP_THRESH_W, value));
}

void FH101RF::getModulationForCorrelationPattern(uint8_t pattern, uint8_t* modulation) {
  uint32_t sequence = correlatorSequences[pattern];

  for(int i = 0; i < 32; i++) {
    modulation[i] = (sequence >> (31 - i) & 0b1);
  }
}

void FH101RF::getModulationForPreamble(uint8_t pattern, uint8_t* modulation) {
  return getModulationForCorrelationPattern(pattern, modulation);
}

void FH101RF::getModulationForPayload(uint8_t patternA, uint8_t patternB, uint8_t* payload, uint8_t* modulation, uint8_t byteLength) {
  uint8_t modulationA[RADIOLIB_FH101RF_SYMBOL_SIZE] = {0};
  uint8_t modulationB[RADIOLIB_FH101RF_SYMBOL_SIZE] = {0};

  getModulationForCorrelationPattern(patternA, modulationA);
  getModulationForCorrelationPattern(patternB, modulationB);

  for(int i = 0; i < byteLength; i++) {
    // get the symbol modulation for every bit of the byte
    for(int j = 0; j < 8; j++) {
      uint8_t* symbolModulation = (payload[i] >> (7 - j) & 0b1) ? modulationB : modulationA;
      memcpy(modulation + (i * 8 + j) * RADIOLIB_FH101RF_SYMBOL_SIZE, symbolModulation, RADIOLIB_FH101RF_SYMBOL_SIZE);
    }
  }
}

uint16_t FH101RF::getSymbolDuration(uint8_t sampleRate) {
  return 1000000 / (32768 / pow(2, sampleRate) * 1.01);
}

#endif
