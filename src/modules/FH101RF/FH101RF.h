/*
  RadioLib Module Template header file

  Before opening pull request, please make sure that:
  1. All files MUST be compiled without errors using default Arduino IDE settings.
  2. All files SHOULD be compiled without warnings with compiler warnings set to "All".
  3. Example sketches MUST be working correctly and MUST be stable enough to run for prolonged periods of time.
  4. Writing style SHOULD be consistent.
  5. Comments SHOULD be in place for the most important chunks of code and SHOULD be free of typos.
  6. To indent, 2 spaces MUST be used.

  If at any point you are unsure about the required style, please refer to the rest of the modules.
*/

#if !defined(_RADIOLIB_FH101RF_H) && !defined(RADIOLIB_EXCLUDE_FH101RF)
#if !defined(_RADIOLIB_FH101RF_H)
#define _RADIOLIB_FH101RF_H

/*
  Header file for each module MUST include Module.h and TypeDef.h in the src folder.
  The header file MAY include additional header files.
*/
#include "../../Module.h"
#include "../../TypeDef.h"

/*
  Only use the following include if the module implements methods for OSI physical layer control.
  This concerns only modules similar to SX127x/RF69/CC1101 etc.

  In this case, your class MUST implement all virtual methods of PhysicalLayer class.
*/
//#include "../../protocols/PhysicalLayer/PhysicalLayer.h"

/*
  Register map
  Definition of SPI register map SHOULD be placed here. The register map SHOULD have two parts:

  1 - Address map: only defines register names and addresses. Register names MUST match names in
      official documentation (datasheets etc.).
  2 - Variable map: defines variables inside register. This functions as a bit range map for a specific register.
      Bit range (MSB and LSB) as well as short description for each variable MUST be provided in a comment.

  See RF69 and SX127x header files for examples of register maps.
*/

// FH101RF register map                                   | spaces up to this point
#define RADIOLIB_FH101RF_REG_NFA433_SLOW                  0x00
#define RADIOLIB_FH101RF_REG_NFA433_FAST                  0x01
#define RADIOLIB_FH101RF_REG_NFA868_SLOW                  0x02
#define RADIOLIB_FH101RF_REG_NFA868_FAST                  0x03
#define RADIOLIB_FH101RF_REG_NFA2G4_SLOW                  0x04
#define RADIOLIB_FH101RF_REG_NFA2G4_FAST                  0x05
#define RADIOLIB_FH101RF_REG_CALIB_STATUS                 0x06
#define RADIOLIB_FH101RF_REG_CALIB_CTRL                   0x07
#define RADIOLIB_FH101RF_REG_N_SPG_TARGET                 0x09
#define RADIOLIB_FH101RF_REG_D_CORNER_CTRL                0x23
#define RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL             0x24
#define RADIOLIB_FH101RF_REG_CODE_SELECT                  0x28
#define RADIOLIB_FH101RF_REG_IRQ_SELECT                   0x31
#define RADIOLIB_FH101RF_REG_IRQ_STATUS                   0x32
#define RADIOLIB_FH101RF_REG_IRQ_CLR                      0x33
#define RADIOLIB_FH101RF_REG_ID_HI                        0x35
#define RADIOLIB_FH101RF_REG_ID_LO                        0x36
#define RADIOLIB_FH101RF_REG_IDM_CTRL                     0x38
#define RADIOLIB_FH101RF_REG_IDM_BAND                     0x3A
#define RADIOLIB_FH101RF_REG_IDM_REASON                   0x3B
#define RADIOLIB_FH101RF_REG_LC_TG_ENA                    0x76
#define RADIOLIB_FH101RF_REG_XTAL_GOOD                    0x77
#define RADIOLIB_FH101RF_REG_COMP_THRESH_W                0x78
#define RADIOLIB_FH101RF_REG_VERSION                      0x7F

// RADIOLIB_FH101RF_REG_CALIB_STATUS
// RADIOLIB_FH101RF_REG_CALIB_CTRL                                      MSB   LSB   DESCRIPTION
#define RADIOLIB_FH101RF_CALIBRATION_ACTIVE               0b00000001  //  0     0   Calibration should be activated or is active, if set
#define RADIOLIB_FH101RF_CALIBRATION_OSCILLATOR           0b00000010  //  1     1   Oscillator calibration
#define RADIOLIB_FH101RF_CALIBRATION_SAMPLE_PULSE         0b00000100  //  2     2   Sample Pulse calibration
#define RADIOLIB_FH101RF_CALIBRATION_COMPARATOR           0b00001000  //  3     3   Comparator Calibration

// RADIOLIB_FH101RF_REG_NFA_xxx_yyyy                                    
#define RADIOLIB_FH101RF_SAMPLE_RATE_32768                0b00000000  //  2     0   Sample Rate: 32768 Hz, Code-Sequence-Duration:   0.977 ms, fastest for mono-band
#define RADIOLIB_FH101RF_SAMPLE_RATE_16384                0b00000001  //  2     0                16384 Hz,                           1.953 ms, fastest for dual-band, fastest for 868 MHz band in tri-band mode
#define RADIOLIB_FH101RF_SAMPLE_RATE_8192                 0b00000010  //  2     0                 8192 Hz,                           3.906 ms, fastest for 433 MHz and 2.4 GHz band in tri-band mode                       
#define RADIOLIB_FH101RF_SAMPLE_RATE_4096                 0b00000011  //  2     0                 4096 Hz,                           7.813 ms
#define RADIOLIB_FH101RF_SAMPLE_RATE_2048                 0b00000100  //  2     0                 2048 Hz,                          15.625 ms
#define RADIOLIB_FH101RF_SAMPLE_RATE_1024                 0b00000101  //  2     0                 1024 Hz,                          31.250 ms, typical
#define RADIOLIB_FH101RF_SAMPLE_RATE_512                  0b00000110  //  2     0                  512 Hz,                          62.500 ms
#define RADIOLIB_FH101RF_SAMPLE_RATE_256                  0b00000111  //  2     0                  256 Hz,                         125.000 ms, slowest

// RADIOLIB_FH101RF_REG_BAND_BRANCH_CTRL                                MSB   LSB   DESCRIPTION
#define RADIOLIB_FH101RF_BRANCH_WEAK_MASK                 0b00000001  //  0     0   Mask for weak branch
#define RADIOLIB_FH101RF_BRANCH_MEDIUM_MASK               0b00000010  //  1     1   Mask for medium branch
#define RADIOLIB_FH101RF_BRANCH_STRONG_MASK               0b00000100  //  2     2   Mask for strong branch
#define RADIOLIB_FH101RF_BAND_433_MASK                    0b00010000  //  4     4   Mask for the 433 MHz band
#define RADIOLIB_FH101RF_BAND_868_MASK                    0b00100000  //  5     5   Mask for the 868 MHz band
#define RADIOLIB_FH101RF_BAND_2G4_MASK                    0b01000000  //  6     6   Mask for the 2G4 MHz band

// RADIOLIB_FH101RF_REG_CODE_SELECT
#define RADIOLIB_FH101RF_CODE_MLS_A                       0x0
#define RADIOLIB_FH101RF_CODE_MLS_B                       0x1
#define RADIOLIB_FH101RF_CODE_MLS_C                       0x2
#define RADIOLIB_FH101RF_CODE_MLS_D                       0x3
#define RADIOLIB_FH101RF_CODE_MLS_A_INV                   0x4
#define RADIOLIB_FH101RF_CODE_MLS_B_INV                   0x5
#define RADIOLIB_FH101RF_CODE_M_SEQUENCE_A                0x6
#define RADIOLIB_FH101RF_CODE_M_SEQUENCE_B                0x7
#define RADIOLIB_FH101RF_CODE_31_ZEROS                    0x8
#define RADIOLIB_FH101RF_CODE_8_ONES                      0x9
#define RADIOLIB_FH101RF_CODE_16_ONES                     0xA
#define RADIOLIB_FH101RF_CODE_24_ONES                     0xB
#define RADIOLIB_FH101RF_CODE_31_ONES                     0xC
#define RADIOLIB_FH101RF_CODE_0101_PATTERN                0xD
#define RADIOLIB_FH101RF_CODE_1100_PATTERN                0xE
#define RADIOLIB_FH101RF_CODE_111000_PATTERN              0xF

// RADIOLIB_FH101RF_REG_IRQ_SELECT
#define RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH                0b00000001  //  0     0   16-bit ID fits to received FDD
#define RADIOLIB_FH101RF_IRQ_TYPE_FIFO_OVERFLOW           0b00000010  //  1     1   FIFO buffer is overflowed   
#define RADIOLIB_FH101RF_IRQ_TYPE_FIFO_BUFFER_FILLED      0b00000100  //  2     2   FIFO buffer is filled
#define RADIOLIB_FH101RF_IRQ_TYPE_CORR_PATTERN_MATCH      0b00001000  //  3     3   OOK data matches the selected correlation sequences (does not depend on FDD or LDR mode being active)
#define RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_FIFO       0b00010000  //  4     4   Type 0 occured within last (FIFO_LENGTH+2) bit cycles and Type 2 occured after Type 0
#define RADIOLIB_FH101RF_IRQ_TYPE_ID_MATCH_AND_LDR        0b00100000  //  5     5   Type 0 occured within current FDD data reception and transition from HDR to LDR sampling mode occured after Type 0
#define RADIOLIB_FH101RF_IRQ_TYPE_RTC_TIMER_ALARM         0b01000000  //  6     6   One RTC timer reached the user-defined target and caused alarm
#define RADIOLIB_FH101RF_IRQ_TYPE_CYCLIC_TIMER_ALARM      0b10000000  //  7     7   Cyclic timer reached user-defined target and caused alarm  

// RADIOLIB_FH101RF_REG_IDM_CTRL
#define RADIOLIB_FH101RF_ID_MATCH_INDIVIDUAL_ONLY         0b00000000  //  1     0   Only individual 16 bit ID
#define RADIOLIB_FH101RF_ID_MATCH_INDIVIDUAL_AND_GROUP    0b00000001  //  1     0   Individual 16 bit ID or groupwise ID
#define RADIOLIB_FH101RF_ID_MATCH_BROADCAST_ONLY          0b00000010  //  1     0   Only broadcast ID
#define RADIOLIB_FH101RF_ID_MATCH_ALL                     0b00000011  //  1     0   Individual 16 bit ID, groupwise ID oder broadcast ID

// RADIOLIB_FH101RF_REG_IDM_BAND
#define RADIOLIB_FH101RF_IDM_BAND_433                     0b00000000  //  2     0   ID match was triggered on 433 MHz band
#define RADIOLIB_FH101RF_IDM_BAND_868                     0b00000001  //  2     0   ID match was triggered on 868 MHz band
#define RADIOLIB_FH101RF_IDM_BAND_2G4                     0b00000010  //  2     0   ID match was triggered on 2400 MHz band
#define RADIOLIB_FH101RF_IDM_BAND_NONE                    0b00000011  //  2     0   ID match has not been triggered yet

// RADIOLIB_FH101RF_REG_IDM_REASON
#define RADIOLIB_FH101RF_IDM_REASON_NONE                  0b00000000  //  2     0   ID match has not been triggered yet
#define RADIOLIB_FH101RF_IDM_REASON_INDIVIDUAL_ID         0b00000001  //  2     0   Individual ID
#define RADIOLIB_FH101RF_IDM_REASON_GROUP_ID              0b00000010  //  2     0   Group ID
#define RADIOLIB_FH101RF_IDM_REASON_BROADCAST_ID          0b00000011  //  2     0   Broadcast ID

// RADIOLIB_FH101RF_REG_VERSION
#define RADIOLIB_FH101RF_VERSION_CURRENT                  0x41

// FH101RF SPI commands
#define RADIOLIB_FH101RF_CMD_READ                         0b10000000
#define RADIOLIB_FH101RF_CMD_WRITE                        0b00000000

// FH101RF Symbol Specifications
#define RADIOLIB_FH101RF_SYMBOL_SIZE                      32
#define RADIOLIB_FH101RF_ID_SYMBOLS                       16
#define RADIOLIB_FH101RF_AMOUNT_CORRELATION_PATTERNS      16

/*
  Module class definition

  The module class MAY inherit from the following classes:

  1 - PhysicalLayer: In case the module implements methods for OSI physical layer control (e.g. SX127x).
  2 - Common class: In case the module further specifies some more generic class (e.g. SX127x/SX1278)
*/
class FH101RF {
  public:
    /*
      Constructor MUST have only one parameter "Module* mod".
      The class MAY implement additional overloaded constructors.
    */
    // constructor
    FH101RF(Module* mod);

    /*
      The class MUST implement at least one basic method called "begin".
      The "begin" method MUST initialize the module and return the status as int16_t type.
    */
    // basic methods
    int16_t begin();

    /*
      The class MAY implement additional methods.
      All implemented methods SHOULD return the status as int16_t type.
    */

    /*!
      \brief Reset method. Will reset the chip to the default state using RST pin.
    */
    void reset();

    /*!
      \brief Read version SPI register. Should return FH101RF_VERSION_CURRENT (0x41) 
      if FH101RF is connected and working.
      \returns Version register contents or \ref status_codes
    */
    int16_t getChipVersion();

    /*!
      \brief Get whether the clock source is stable. This is mandatory for proper receiver operation.
      \returns 1 if clock source is stable, 0 if not or \ref status_codes.
    */
    int16_t isClockSourceStable();

    /*!
      \brief Sets D_CORNER_CTRL. This needs to be set to 0x02 during power up.
      \param value Value to be set, needs to be 0x02 during power up.
      \returns \ref status_codes
    */
    int16_t setDCornerCtrl(uint8_t value);

    /*!
      \brief Sets LC_TG_ENA. This needs to be set to 0x00 during power up.
      \param value Value to be set, needs to be 0x00 during power up.
      \returns \ref status_codes
    */
    int16_t setLcTgEna(uint8_t value);

    /*!
      \brief Calibrates local oscillator, sample pulse and comparator.
      \returns \ref status_codes
    */
    int16_t calibrate();

    /*!
      \brief Sets the state of the individual bands.
      \param band433 Whether the 433 MHz band should be active or not.
      \param band868 Whether the 868 MHz band should be active or not.
      \param band2G4 Whether the 2.4 GHz band should be active or not.
      \returns \ref status_codes  
    */
    int16_t setActiveBands(bool band433, bool band868, bool band2G4);

    /*!
      \brief Sets the state of the individual branches.
      \param branchWeak Whether the weak branch should be active or not.
      \param branchMedium Whether the medium branch should be active or not.
      \param branchStrong Whether the strong branch should be active or not.
      \returns \ref status_codes  
    */
    int16_t setActiveBranches(bool branchWeak, bool branchMedium, bool branchStrong);


    /*!
      \brief Sets the sample rates of the LDR and HDR modes.
      \param srPreamble The sample rate of the LDR mode (preamble).
      \param srFastRx The sample rate of the HDR mode (fast RX).
      \returns \ref status_codes  
    */
    int16_t setSampleRate(uint8_t srPreamble, uint8_t srFastRx);

    /*!
      \brief Sets the IRQ mode.
      \param mode Bit mask for IRQ mode.
      \returns \ref status_codes  
    */
    int16_t setIrqMode(uint8_t mode);

    /*!
      \brief Gets the current IRQ status.
      \returns \ref IRQ status or \ref status_codes
    */
    int16_t getIrqStatus();

    /*!
      \brief Resets the IRQ status.
      \param mode Bit mask for IRQ mode(s) to be reset.
      \returns \ref status_codes
    */
    int16_t resetIrqStatus(uint8_t mode);

    /*!
      \brief Selects the ID match mode for creating the wake-up signal.
      \param mode ID match mode
      \returns \ref status_codes  
    */
    int16_t setIdMatchMode(uint8_t mode);

    /*!
      \brief Sets the individual ID of the receiver.
      \param id The ID of the receiver.
      \returns status_codes
    */
    int16_t setReceiverId(uint16_t id);

    /*!
      \brief Selects the bit sequences that represent "code A" and "code B" in the wake-up protocol.
      \param codeA Index of bit sequence for "code A"
      \param codeB Index of bit sequence for "code B"
      \returns \ref status_codes
    */
    int16_t setCorrelationPatterns(uint8_t codeA, uint8_t codeB);
    
    static void getModulationForPreamble(uint8_t pattern, uint8_t* symbol);
    static void getModulationForPayload(uint8_t patternOne, uint8_t patternZero, uint8_t* payload, uint8_t* symbols, uint8_t byteLength);
    static uint16_t getSymbolDuration(uint8_t sampleRate);

#if !defined(RADIOLIB_GODMODE)
  private:
#endif
    /*
      The class MUST contain private member "Module* _mod"
    */
    Module* _mod;

    /*
      The class MAY contain additional private variables and/or methods.
      Private member variables MUST have a name prefixed with "_" (underscore, ASCII 0x5F)

      Usually, these are variables for saving module configuration, or methods that do not have to be exposed to the end user.
    */
    int16_t setBandBranchControlRaw(uint8_t value);
    int16_t startCalibration(uint8_t value);
    
    bool isCalibrationStarted();
    bool isCalibrationRunning();

    int16_t calibrateLocalOscillator();
    int16_t calibrateSamplePulse();
    int16_t calibrateComparator();

    int16_t setComparatorThreshold(uint8_t value);

    static void getModulationForCorrelationPattern(uint8_t pattern, uint8_t* modulation);
};

#endif

#endif
