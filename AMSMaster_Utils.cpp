#include "AMSMaster_Utils.hpp"

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
    ResetAllFaults(DEVICE, FRMWRT_ALL_W);
}

void readCells(int device, int totalBoards, CellVoltage cellData[]) {
    uint16_t raw_data = 0;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];
    
    SpiReadReg(device, (VCELL16_HI+(16-ACTIVECHANNELS)*2), response_frame, ACTIVECHANNELS*2, 0, FRMWRT_STK_R);
    
    for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {

        for (int channel = 0; channel < ACTIVECHANNELS*2; channel += 2) {
            int boardStart = (ACTIVECHANNELS*2*+6) * currBoard;
            
            raw_data = (response_frame[channel + boardStart + 4] << 8) | response_frame[channel + boardStart + 5];
            float cell_voltage = raw_data * 0.00019073; 
            SerialUSB.println(cell_voltage);
            SerialUSB.println(channel/2 + 1);
            cellData[currBoard*ACTIVECHANNELS + channel] = {channel/2 + 1, cell_voltage};
        }
    }


} 

/**
 * @brief Reads the cell temperatures from the bq79600. The temperature is calculated using Linear Interpolation. 
 * 
 * @param cellData vector with struct CellVoltage representing the cell and its voltage
 * 
*/
CellTemperature * calculateCellTemperatures(CellVoltage* cellData) {
    CellTemperature * tempData;  

    double xValues[33] = {1.30, 1.31, 1.32, 1.33, 1.34, 1.35, 1.37, 1.38, 1.40, 1.43, 1.45, 1.48, 1.51, 1.55, 1.59, 1.63, 1.68, 1.74, 1.80, 1.86, 1.92, 1.99, 2.05, 2.11, 2.17, 2.23, 2.27, 2.32, 2.35, 2.38, 2.40, 2.42, 2.44};
    double yValues[33] = {120, 115, 110, 105, 100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0, -5, -10, -15, -20, -25, -30, -35, -40};

    for (int i = 0; i < sizeof(cellData)/ sizeof(cellData[0]); i++) {
        double temp = Interpolation::Linear(xValues, yValues, 33, cellData[i].voltage, true);
        tempData[i] = {cellData[i].channel, temp};
    }
   
    return tempData;
}

int getFaultData(int device, int faultRegister) {
    uint16_t fault_response_frame[FAULT_FRAME_SIZE];
    SpiReadReg(device, faultRegister, fault_response_frame, 1, 0, FRMWRT_STK_R);
    return fault_response_frame[4];
}

FAULTS * readFaults(int device, int totalBoards, int faultRegister) { 
    int rawData = getFaultData(device, faultRegister);   
    static FAULTS faultData[TOTALFAULT_BIT];
    memset(faultData, 0, sizeof(faultData));

    for (int i = 0; i < TOTALFAULT_BIT; i++) {
        if (rawData & (1 << i)) {
            faultData[i] = static_cast<FAULTS>(i);
        }
    }

    return faultData;
}

float complement(uint16_t raw_data) {
    return -1*(~raw_data+1);
}

String getFaultSummaryString(FAULTS fault) {
    switch(fault) {
        case FAULT_PWR_:
            return "FAULT_PWR";
        case FAULT_SYS_:
            return "FAULT_SYS";
        case FAULT_OVUV_:
            return "FAULT_OVUV";
        case FAULT_OTUT_:
            return "FAULT_OTUT";
        case FAULT_COMM_:
            return "FAULT_COMM";
        case FAULT_OTP_:
            return "FAULT_OTP";
        case FAULT_COMP_ADC_:
            return "FAULT_COMP_ADC";
        case FAULT_PROT_:
            return "FAULT_PROT";
        default:
            return "FAULT_UNKNOWN";    
    }
}

String getFaultString(FAULTS highLevelFault, int lowLevelFault) {
    switch(highLevelFault) {
        case FAULT_PWR_:
            switch(lowLevelFault) {
                case FAULT_PWR1:
                    return "FAULT_PWR1";
                case FAULT_PWR2:
                    return "FAULT_PWR2";
                case FAULT_PWR3:
                    return "FAULT_PWR3";
                default:
                    return "FAULT_PWR_UNKNOWN";
            }
        case FAULT_SYS_:
            switch (lowLevelFault) {
                case FAULT_SYS:
                    return "FAULT_SYS";
                default:
                    return "FAULT_SYS_UNKNOWN";
            }
        case FAULT_OVUV_:
            switch(lowLevelFault) {
                case FAULT_OV1:
                    return "FAULT_OV1";
                case FAULT_OV2:
                    return "FAULT_OV2";
                case FAULT_UV1:
                    return "FAULT_UV1";
                case FAULT_UV2:
                    return "FAULT_UV2";
                default:
                    return "FAULT_OVUV_UNKNOWN";
            }
        case FAULT_OTUT_:
            switch(lowLevelFault) {
                case FAULT_OT:
                    return "FAULT_OT";
                case FAULT_UT:
                    return "FAULT_UT";
                default:
                    return "FAULT_OTUT_UNKNOWN";
            }
        case FAULT_COMM_:
            switch(lowLevelFault) {
                case FAULT_COMM1:
                    return "FAULT_COMM1";
                case FAULT_COMM2:
                    return "FAULT_COMM2";
                case FAULT_COMM3:
                    return "FAULT_COMM3";
                default:
                    return "FAULT_COMM_UNKNOWN";
            }
        case FAULT_OTP_:
            switch(lowLevelFault) {
                case FAULT_OTP:
                    return "FAULT_OTP";
                default:
                    return "FAULT_OTP_UNKNOWN";
            }
        case FAULT_COMP_ADC_:
            switch(lowLevelFault) {
                case FAULT_COMP_GPIO:
                    return "FAULT_COMP_GPIO";
                case FAULT_COMP_VCCB1:
                    return "FAULT_COMP_VCCB1";
                case FAULT_COMP_VCCB2:
                    return "FAULT_COMP_VCCB2";
                case FAULT_COMP_VCOW1:
                    return "FAULT_COMP_VCOW1";
                case FAULT_COMP_VCOW2:
                    return "FAULT_COMP_VCOW2";
                case FAULT_COMP_CBOW1:
                    return "FAULT_COMP_CBOW1";
                case FAULT_COMP_CBOW2:
                    return "FAULT_COMP_CBOW2";
                case FAULT_COMP_CBFET1:
                    return "FAULT_COMP_CBFET1";
                case FAULT_COMP_CBFET2:
                    return "FAULT_COMP_CBFET2";
                case FAULT_COMP_MISC:
                    return "FAULT_COMP_MISC";
                default:
                    return "FAULT_COMP_ADC_UNKNOWN";
            }
        case FAULT_PROT_:
            switch(lowLevelFault) {
                case FAULT_PROT1:
                    return "FAULT_PROT1";
                case FAULT_PROT2:
                    return "FAULT_PROT2";
                default:
                    return "FAULT_PROT_UNKNOWN";
            }
        default:
            return "FAULT_UNKNOWN";
    }
}

int * getLowLevelFaultRegisters(FAULTS fault) {
    static int faultRegisters[10];
    memset(faultRegisters, 0, sizeof(faultRegisters));

    switch(fault) {
        case FAULT_PWR_:
            faultRegisters[0] = FAULT_PWR1;
            faultRegisters[1] = FAULT_PWR2;
            faultRegisters[2] = FAULT_PWR3;
            break;
        case FAULT_SYS_:
            faultRegisters[0] = FAULT_SYS;
            break;
        case FAULT_OVUV_:
            faultRegisters[0] = FAULT_OV1;
            faultRegisters[1] = FAULT_OV2;
            faultRegisters[2] = FAULT_UV1;
            faultRegisters[3] = FAULT_UV2;
            break;
        case FAULT_OTUT_:
            faultRegisters[0] = FAULT_OT;
            faultRegisters[1] = FAULT_UT;
            break;
        case FAULT_COMM_:
            faultRegisters[0] = FAULT_COMM1;
            faultRegisters[1] = FAULT_COMM2;
            faultRegisters[2] = FAULT_COMM3;
            break;
        case FAULT_OTP_:
            faultRegisters[0] = FAULT_OTP;
            break;
        case FAULT_COMP_ADC_:
            faultRegisters[0] = FAULT_COMP_GPIO;
            faultRegisters[1] = FAULT_COMP_VCCB1;
            faultRegisters[2] = FAULT_COMP_VCCB2;
            faultRegisters[3] = FAULT_COMP_VCOW1;
            faultRegisters[4] = FAULT_COMP_VCOW2;
            faultRegisters[5] = FAULT_COMP_CBOW1;
            faultRegisters[6] = FAULT_COMP_CBOW2;
            faultRegisters[7] = FAULT_COMP_CBFET1;
            faultRegisters[8] = FAULT_COMP_CBFET2;
            faultRegisters[9] = FAULT_COMP_MISC;
            break;
        case FAULT_PROT_:
            faultRegisters[0] = FAULT_PROT1;
            faultRegisters[1] = FAULT_PROT2;
            break;
        default:
            break;    
    }


    return faultRegisters;
}


    