#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <vector>

// Check and undefine min and max macros
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <algorithm> // Ensure this is included after undefining macros

#include <bq79616.hpp>

#define ARDUINO_DUE_BAUDRATE    9600
#define DEVICE                  0

struct CellVoltage {
    int channel;
    float voltage;
};

struct DIETemperature {
    int die;
    float temperature;
};

enum FAULTS {
    FAULT_PWR_,
    FAULT_SYS_,
    FAULT_REG_,
    FAULT_COMM_,
};

float complement(uint16_t raw_data);
void wakeSequence();
std::vector<CellVoltage> readCells(int device, int totalBoards);
std::vector<DIETemperature> readTemperatures(int device, int totalBoards);
std::vector<FAULTS> readFaults(int device, int totalBoards);


#endif // AMSMASTER_UTILS_HPP_
//EOF
