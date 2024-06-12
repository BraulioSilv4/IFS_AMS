#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include "vector"
#include <InterpolationLib.h>
#include "bq79616.hpp"

#define ARDUINO_DUE_BAUDRATE    9600
#define DEVICE                  0
#define TOTALFAULT_BIT          8

#define length(array) (sizeof(array) / sizeof(array[0]))

struct CellVoltage {
    int channel;
    float voltage;
};

struct CellTemperature {
    int channel;
    double temperature;
};

struct FAULT_DATA {
    int fault;
    int data;
};

enum FAULTS {
    FAULT_PWR_,
    FAULT_SYS_,
    FAULT_OVUV_,
    FAULT_OTUT_,
    FAULT_COMM_,
    FAULT_OTP_,
    FAULT_COMP_ADC_,
    FAULT_PROT_
};

String getFaultSummaryString(FAULTS fault);
String getFaultString(FAULTS highLevelFault, int lowLevelFault);
float complement(uint16_t raw_data);
void wakeSequence();
void readCells(int device, int totalBoards, CellVoltage cellData[]);
CellTemperature* calculateCellTemperatures(CellVoltage* cellData);
FAULTS* readFaults(int device, int totalBoards, int faultRegister = FAULT_SUMMARY);
int * getLowLevelFaultRegisters(FAULTS fault);
int getFaultData(int device, int faultRegister);

#endif // AMSMASTER_UTILS_HPP_
//EOF
