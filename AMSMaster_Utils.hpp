#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <InterpolationLib.h>
#include "bq79616.hpp"
#include <due_can.h>

#define ARDUINO_DUE_BAUDRATE    9600
#define DEVICE                  0
#define TOTALFAULT_BIT          8
#define MAX_LOW_LEVEL_FAULTS    10
#define OVER_TEMPERATURE        1
#define UNDER_TEMPERATURE       0
#define OVER_VOLTAGE           1
#define UNDER_VOLTAGE          0
#define VOLTAGE_FAULT_ID        0x64
#define TEMPERATURE_FAULT_ID    0x65
#define FAULT_ID                0x66

#define length(array) (sizeof(array) / sizeof(array[0]))

struct CellVoltage {
    int channel;
    float voltage;
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

struct FAULT_DATA {
    FAULT topLevelFault;
    uint16_t lowLevelFaults[MAX_LOW_LEVEL_FAULTS];
    size_t size;
};

struct BOARD_FAULT_SUMMARY {
    int board;
    uint8_t faultSummary;
}; 

float complement(uint16_t raw_data);
void wakeSequence();
void readCells(int device, int totalBoards, int channels, int reg, CellVoltage cellData[]);
void calculateCellTemperatures(CellVoltage cellData[], CellTemperature cellTempData[], int length);
void readFaultSummary(BOARD_FAULT_SUMMARY boardsFaultSummary[]);
void sendFaultFrames(BOARD_FAULT_SUMMARY boardFaultSummary[]);

#endif // AMSMASTER_UTILS_HPP_
//EOF
