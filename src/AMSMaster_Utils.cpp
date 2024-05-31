#include <AMSMaster_Utils.hpp>

void sendWakeTone() {
    SpiWriteReg(0, CONTROL1, 0x20, 1, FRMWRT_SGL_W); //send wake tone to stack devices
    delayMicroseconds((10000 + 600) * TOTALBOARDS); //wake tone duration is ~1.6ms per board + 10ms per board for each device to wake up from shutdown = 11.6ms per 616 board.
}

void activateCells() {
    // Activate cells
    SpiWriteReg(1, ACTIVE_CELL, 0x0A, 1, FRMWRT_STK_W);
    // Set the ADC run mode to continuous
    SpiWriteReg(1, ADC_CTRL1, 0x06, 1, FRMWRT_STK_W);
    // Wait the required round robin time
    delayMicroseconds(192 + (5 * TOTALBOARDS)); 
}

void setPins() {
    pinMode(nCS, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SPI_RDY, INPUT);

    digitalWrite(nCS, HIGH);
    digitalWrite(MOSI, HIGH);
}

void wakeSequence() {
    setPins();
    SPI.setClockDivider(SPI_CLOCK_DIV16);

    // Wake Sequence
    // two wakes in case the MCU had nCS and MOSI = 0 at start (which would put the device in shutdown) or in case the device was previously put in shutdown through a shutdown ping
    SpiWake79600(); //send wake ping to bridge device
    delayMicroseconds(3500); //wait tSU(WAKE_SHUT), at least 3.5ms
    SpiWake79600(); //send wake ping to bridge device
    delayMicroseconds(3500); //tSU(WAKE_SHUT), at least 3.5ms

    SPI.begin();
    sendWakeTone();
    SpiAutoAddress();
    activateCells();
}

std::vector<CellVoltage> readCells(int device, int totalBoards) {
    uint16_t raw_data = 0;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];
    std::vector<CellVoltage> cellData; 
    
    SpiReadReg(device, (VCELL16_HI+(16-ACTIVECHANNELS)*2), response_frame, ACTIVECHANNELS*2, 0, FRMWRT_STK_R);
    
    for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {

        for (int channel = 0; channel < ACTIVECHANNELS*2; channel += 2) {
            int boardStart = (ACTIVECHANNELS*2*+6) * currBoard;
            
            raw_data = (response_frame[channel + boardStart + 4] << 8) | response_frame[channel + boardStart + 5];
            float cell_voltage = raw_data * 0.00019073; 
 
            cellData.push_back({channel/2 + 1, cell_voltage});    
        }
    }

    return cellData;
} 

// std::vector<DIETemperature> readTemperatures(int device, int totalBoards) {
//     uint16_t raw_data = 0;
//     uint16_t temp_response_frame[TEMP_FRAME_SIZE];
//     std::vector<DIETemperature> tempData;

//     SpiReadReg(device, DIETEMP2_HI, temp_response_frame, CELL_TEMP_NUM*2, 0, FRMWRT_STK_R);

//     for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {
//         for (int die = 0; die < CELL_TEMP_NUM*2; die += 2) {
//             int boardStart = (CELL_TEMP_NUM*2*+6) * currBoard;
            
//             raw_data = (temp_response_frame[die + boardStart + 4] << 8) | temp_response_frame[die + boardStart + 5];
//             float die_temp = raw_data * 0.00390625;

//             tempData.push_back({die/2 + 1, die_temp});
//         }
//     }

//     return tempData;
// }

std::vector<FAULTS> readFaults(int device, int totalBoards) {
    uint16_t raw_data = 0;
    uint16_t fault_response_frame[FAULT_FRAME_SIZE];
    std::vector<FAULTS> faultData;

    SpiReadReg(device, FAULT_SUMMARY, fault_response_frame, 1, 0, FRMWRT_STK_R);

    
}

float complement(uint16_t raw_data) {
    return -1*(~raw_data+1);
}
    