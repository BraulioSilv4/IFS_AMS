#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <InterpolationLib.h>
#include "bq79616.hpp"


#define ARDUINO_DUE_BAUDRATE    9600
#define FAULT_TIMEOUT           500
#define TEMP_FAULT_TIMEOUT      1000
#define VOLTAGE_FAULT_TIMEOUT   500
#define UPDATE_INTERVAL         500
#define READ_FAULT_INTERVAL     100
#define FAULT_UPDATE_INTERVAL   500
#define BAL_TIME                20000

#define TEMPERATURE_SCALING_FACTOR 100
#define DEVICE                  0
#define TOTALFAULT_BIT          8
#define MAX_LOW_LEVEL_FAULTS    10
#define OVER_TEMPERATURE        1
#define UNDER_TEMPERATURE       0
#define OVER_VOLTAGE           1
#define UNDER_VOLTAGE          0
#define VOLTAGE_FAULT_ID        0x12D
#define TEMPERATURE_FAULT_ID    0x12E
#define FAULT_ID                0x12F
#define COMM_FAULT_ID           0x154
#define AR_STATE_FRAME_ID       0x155
#define BASE_VOLTAGE_FRAME_ID        0x130
#define BASE_TEMPERATURE_FRAME_ID    0x148
#define INCOMING_CB_FRAME_ID        0x12B
#define BASE_CB_FRAME_ID            0x12C

#define length(array) (sizeof(array) / sizeof(array[0]))

struct CellVoltage {
    int board;
    int channel;
    uint16_t rawVoltage;
};

struct CellTemperature {
    int channel;
    float temperature;
};

enum FAULT {
    FAULT_PWR_,
    FAULT_SYS_,
    FAULT_OVUV_,
    FAULT_OTUT_,
    FAULT_COMM_,
    FAULT_OTP_,
    FAULT_COMP_ADC_,
    FAULT_PROT_
};

struct FAULT_INFO {
    FAULT topLevelFault;
    uint16_t lowLevelFaults[MAX_LOW_LEVEL_FAULTS];
    size_t size;
    int rstReg;
    int rstVal;
};

struct BOARD_FAULT {
    FAULT fault;
    uint32_t timeout;
    bool hasTimeoutStarted;
    bool hasTimeoutEnded;
};

struct BOARD_FAULTS_DATA {
    int boardID; 
    BOARD_FAULT faults[TOTALFAULT_BIT]; 
};

struct BOARD_FAULT_SUMMARY {
    int board;
    uint8_t faultSummary;
}; 

void wakeSequence();

void sendCommFaultFrame();
void sendCellBalacingFrames(bool hasFinishedBalancing);
void initializeInternalStructures();
void readCells(int channels, int reg, int frameSize, CellVoltage cellData[]);
void readGPIOS(int channels, int reg, int frameSize, CellVoltage cellData[]);
void calculateCellTemperatures(CellVoltage cellData[], CellTemperature cellTempData[], int length);
void readFaultSummary();
void sendFaultFrames();
void sendVoltageFrames(CellVoltage cellData[]);
void sendTemperatureFrames(CellTemperature cellTempData[]);
void sendARStateFrame();
void balanceCells(double thresh);

#endif // AMSMASTER_UTILS_HPP_
//EOF
