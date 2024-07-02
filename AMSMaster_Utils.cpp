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

void setRegisters() {
    // CONFIGURE GPIOS as temp inputs
    // SpiWriteReg(0, GPIO_CONF1, 0x08, 1, FRMWRT_ALL_W); // GPIO1 and 2 as temp inputs
    //   WriteReg(0, GPIO_CONF2, 0x09, 1, FRMWRT_ALL_W); // GPIO3 and 4 as temp inputs
    //   WriteReg(0, GPIO_CONF3, 0x09, 1, FRMWRT_ALL_W); // GPIO5 and 6 as temp inputs
    //   WriteReg(0, GPIO_CONF4, 0x09, 1, FRMWRT_ALL_W); // GPIO7 and 8 as temp inputs

    SpiWriteReg(DEVICE, OV_THRESH, 0x02, 1, FRMWRT_ALL_W); // Sets Over voltage protection to 4.25V
    SpiWriteReg(DEVICE, UV_THRESH, 0x00, 1, FRMWRT_ALL_W); // Sets Under voltage protection to 3.0V
    SpiWriteReg(DEVICE, OVUV_CTRL, 0x05, 1, FRMWRT_ALL_W); // Sets voltage controls   
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
    ResetAllFaults(DEVICE, FRMWRT_ALL_W);
    setRegisters();
    activateCells();
    ResetAllFaults(DEVICE, FRMWRT_ALL_W);
}

CellVoltage readCell(int device, int reg, int channel) {
    CellVoltage cellData;
    uint16_t raw_data = 0;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];

    SpiReadReg(device, reg, response_frame, 2, 0, FRMWRT_SGL_R);

    raw_data = (response_frame[4] << 8) | response_frame[5];
    float cell_voltage = raw_data * 0.00019073;
    cellData.channel = channel;
    cellData.voltage = cell_voltage;

    return cellData;
}

void readCells(int device, int totalBoards, int channels, int reg, CellVoltage cellData[]) {
    uint16_t raw_data = 0;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];

    SpiReadReg(device, (reg+(16-channels)*2), response_frame, channels*2, 0, FRMWRT_STK_R);

    for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {

        for (int channel = 0; channel < channels*2; channel += 2) {
            int boardStart = (channels*2*+6) * currBoard;

            raw_data = (response_frame[channel + boardStart + 4] << 8) | response_frame[channel + boardStart + 5];
            float cell_voltage = raw_data * 0.00019073;
            int idx = currBoard*channels + channel/2;
            cellData[idx].channel = channel/2 + 1;
            cellData[idx].voltage = cell_voltage;
        }
    }
}



/**
 * @brief Reads the cell temperatures from the bq79600. The temperature is calculated using Linear Interpolation.
 *
 * @param cellData vector with struct CellVoltage representing the cell and its voltage
 *
*/
void calculateCellTemperatures(CellVoltage cellData[], CellTemperature cellTempData[], int length) {
    double xValues[33] = {1.30, 1.31, 1.32, 1.33, 1.34, 1.35, 1.37, 1.38, 1.40, 1.43, 1.45, 1.48, 1.51, 1.55, 1.59, 1.63, 1.68, 1.74, 1.80, 1.86, 1.92, 1.99, 2.05, 2.11, 2.17, 2.23, 2.27, 2.32, 2.35, 2.38, 2.40, 2.42, 2.44};
    double yValues[33] = {120, 115, 110, 105, 100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0, -5, -10, -15, -20, -25, -30, -35, -40};
    for (int i = 0; i < length; i++) {
        double temp = Interpolation::Linear(xValues, yValues, 33, cellData[i].voltage, true);
        cellTempData[i].channel = cellData[i].channel;
        cellTempData[i].temperature = static_cast<float>(temp);
    }
}

FAULT_DATA FaultInfo[TOTALFAULT_BIT] = {
    {
        FAULT_PWR_,
        {FAULT_PWR1, FAULT_PWR2, FAULT_PWR3},
        3, 
        FAULT_RST1,
        0x01, 
        0,
        false,
    },
    {
        FAULT_SYS_,
        {FAULT_SYS},
        1, 
        FAULT_RST1,
        0x02,
        0,
        false,
    },
    {
        FAULT_OVUV_,
        {FAULT_OV1, FAULT_OV2, FAULT_UV1, FAULT_UV2},
        4,
        FAULT_RST1,
        0x18,
        0,
        false,
    },
    {
        FAULT_OTUT_,
        {FAULT_OT, FAULT_UT},
        2,
        FAULT_RST1,
        0x60,
        0,
        false,
    },
    {
        FAULT_COMM_, 
        {FAULT_COMM1, FAULT_COMM2, FAULT_COMM3}, 
        3,
        FAULT_RST2,
        0x1F,
        0,
        false,
    },
    {
        FAULT_OTP_, 
        {FAULT_OTP},
        1,
        FAULT_RST2,
        0x60,
        0,
        false
    },
    {
        FAULT_COMP_ADC_,
        {FAULT_COMP_GPIO, FAULT_COMP_VCCB1, FAULT_COMP_VCCB2, FAULT_COMP_VCOW1, FAULT_COMP_VCOW2, FAULT_COMP_CBOW1, FAULT_COMP_CBOW2, FAULT_COMP_CBFET1, FAULT_COMP_CBFET2, FAULT_COMP_MISC},
        10,
        FAULT_RST1,
        0x04,
        0,
        false
    },
    {
        FAULT_PROT_,
        {FAULT_PROT1, FAULT_PROT2},
        2,
        FAULT_RST1,
        0x80,
        0,
        false
    }
};

void resetClearedFaults(BOARD_FAULT_SUMMARY boardFaultSummary) {
    for (size_t i = 0; i < TOTALFAULT_BIT; i++) {
        FAULT fault = static_cast<FAULT>(i);
        if (!(boardFaultSummary.faultSummary & (1 << i))) {
            FaultInfo[fault].hasTimeoutStarted = false;
        }
    }
}

void readFaultSummary(BOARD_FAULT_SUMMARY boardsFaultSummary[]) {
    uint16_t fault_response_frame[FAULT_FRAME_SIZE];
    SpiReadReg(DEVICE, FAULT_SUMMARY, fault_response_frame, 1, 0, FRMWRT_STK_R);

    for(int currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {
        boardsFaultSummary[currBoard].board = currBoard+1;
        boardsFaultSummary[currBoard].faultSummary = static_cast<uint8_t>(fault_response_frame[4+(currBoard*7)]);
        resetClearedFaults(boardsFaultSummary[currBoard]);       
    }
}

void sendTemperatureFaultFrame(int device, int reg, uint16_t fault_response_frame[], int faultType) {
    SpiReadReg(device, reg, fault_response_frame, 1, 0, FRMWRT_SGL_R);
    uint16_t t = fault_response_frame[4];
    if (t != 0) {
        int baseReg = GPIO8_HI;
        for (size_t i = 0; i < TOTALFAULT_BIT; i++) {
            if (t & (1 << i)) {
                CellVoltage GPIOdata = readCell(device, baseReg + i*2, i);
                CellTemperature cellTempData[1];
                calculateCellTemperatures(&GPIOdata, cellTempData, 1);
                float GPIOtemp = cellTempData[0].temperature;

                CAN_FRAME myCANFrame;
                myCANFrame.id = TEMPERATURE_FAULT_ID;
                myCANFrame.length = 7;
                myCANFrame.data.byte[0] = device;
                myCANFrame.data.byte[1] = i;
                myCANFrame.data.byte[2] = faultType;
                myCANFrame.data.byte[3] = static_cast<int>(GPIOtemp) & 0xFF;
                myCANFrame.data.byte[4] = (static_cast<int>(GPIOtemp) >> 8) & 0xFF;
                myCANFrame.data.byte[5] = (static_cast<int>(GPIOtemp) >> 16) & 0xFF;;
                myCANFrame.data.byte[6] = (static_cast<int>(GPIOtemp) >> 24) & 0xFF;

                CAN.sendFrame(myCANFrame);
            }
        }
    }
}

void sendVoltageFaultFrame(int device, int reg, uint16_t fault_response_frame[], int faultType) {
    SpiReadReg(device, reg, fault_response_frame, 2, 0, FRMWRT_SGL_R);
    uint16_t v = fault_response_frame[4] << 8 | fault_response_frame[5];

    SerialUSB.println(faultType);
    SerialUSB.println(v, BIN);
    if (v != 0) {
        int baseReg = VCELL16_HI;
        for (size_t i = 0; i < TOTALFAULT_BIT*2; i++) {
            if (v & (1 << i)) {
                CellVoltage cellData = readCell(device, baseReg + i*2, i);
                CAN_FRAME myCANFrame;
                myCANFrame.id = VOLTAGE_FAULT_ID;
                myCANFrame.length = 7;
                myCANFrame.data.byte[0] = device;
                myCANFrame.data.byte[1] = i;
                myCANFrame.data.byte[2] = faultType;
                myCANFrame.data.byte[3] = static_cast<int>(cellData.voltage) & 0xFF;
                myCANFrame.data.byte[4] = (static_cast<int>(cellData.voltage) >> 8) & 0xFF;
                myCANFrame.data.byte[5] = (static_cast<int>(cellData.voltage) >> 16) & 0xFF;
                myCANFrame.data.byte[6] = (static_cast<int>(cellData.voltage) >> 24) & 0xFF;

                CAN.sendFrame(myCANFrame);
            }
        }
    }
}

void sendFaultFrame(int device, FAULT fault, uint16_t fault_response_frame[]) {
    for (size_t i = 0; i < FaultInfo[fault].size; i++) {
        int reg = FaultInfo[fault].lowLevelFaults[i];
        SpiReadReg(device, reg, fault_response_frame, 1, 0, FRMWRT_SGL_R);
        uint16_t t = fault_response_frame[4];
        if (t != 0) {
            CAN_FRAME myCANFrame;
            myCANFrame.id = FAULT_ID;
            myCANFrame.length = 4;
            myCANFrame.data.byte[0] = device;
            myCANFrame.data.byte[1] = fault;
            myCANFrame.data.byte[2] = i;
            myCANFrame.data.byte[3] = t;
            CAN.sendFrame(myCANFrame);
        }
    }
}


void sendFaultFrames(BOARD_FAULT_SUMMARY boardsFaultSummary[]) {
    uint16_t fault_response_frame[FAULT_FRAME_SIZE];
    for(size_t currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {
        uint8_t faultSummary = boardsFaultSummary[currBoard].faultSummary;
        int board = boardsFaultSummary[currBoard].board;
        if (faultSummary != 0) {
            for (size_t i = 0; i < TOTALFAULT_BIT; i++){
                FAULT fault = static_cast<FAULT>(i);
                if (faultSummary & (1 << i)) {
                    switch (fault) {
                        case FAULT_OTUT_:
                            sendTemperatureFaultFrame(board, FAULT_OT, fault_response_frame, OVER_TEMPERATURE);
                            sendTemperatureFaultFrame(board, FAULT_UT, fault_response_frame, UNDER_TEMPERATURE);
                            break;
                        case FAULT_OVUV_:
                            sendVoltageFaultFrame(board, FAULT_OV1, fault_response_frame, OVER_VOLTAGE);
                            sendVoltageFaultFrame(board, FAULT_UV1, fault_response_frame, UNDER_VOLTAGE);
                            break;   
                        default:
                            sendFaultFrame(board, fault, fault_response_frame);
                            break;
                    }

                    if (!FaultInfo[fault].hasTimeoutStarted) {
                        SerialUSB.println("\nPRIMEIRO IF");
                        FaultInfo[fault].timeout = millis();
                        FaultInfo[fault].hasTimeoutStarted = true;
                    }

                    if (millis() - FaultInfo[fault].timeout <= FAULT_TIMEOUT){
                        SpiWriteReg(board, FaultInfo[fault].rstReg, FaultInfo[fault].rstVal, 1, FRMWRT_SGL_W); // reset fault
                    } else {
                        while (true)
                        {
                            SerialUSB.println("\nSEGUNDO IF");
                        }
                        
                        digitalWrite(nFAULT, LOW);
                    }    
                }
            }
        }
    }
}

float complement(uint16_t raw_data) {
     return -1*(~raw_data+1);
}



























// void getFaultData(int device, int totalBoards, int faultRegister, FAULT_DATA faultData[]) {
//     uint16_t fault_response_frame[FAULT_FRAME_SIZE];
//     SpiReadReg(device, faultRegister, fault_response_frame, 1, 0, FRMWRT_STK_R);

//     for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {
//         uint8_t fault = static_cast<uint8_t>(fault_response_frame[4+(currBoard*7)]);
//     }
// }

// void readFaultSummary(int device, int totalBoards, BOARD_FAULT_SUMMARY boardFaultSummary[]) {
//     FAULT_DATA faultData[TOTALFAULT_BIT];
//     getFaultData(device, totalBoards, FAULT_SUMMARY, boardFaultSummary);
//     static FAULTS faultData[TOTALFAULT_BIT];
//     memset(faultData, 0, sizeof(faultData));
//     for(int currBoard = 0; currBoard < totalBoards-1; currBoard++) {
//         uint8_t fault_summary = boardFaultSummary[currBoard].fault_summary;
//         for(int i = 0; i < TOTALFAULT_BIT; i++) {
//             if(fault_summary & (1 << i)) {
//                 faultData[i].fault = static_cast<FAULTS>(i);
//                 faultData[i].board = currBoard;
//             }
//         }
//     }
// }


// void readFaults( ) {

// }



// String getFaultSummaryString(FAULTS fault) {
//     switch(fault) {
//         case FAULT_PWR_:
//             return "FAULT_PWR";
//         case FAULT_SYS_:
//             return "FAULT_SYS";
//         case FAULT_OVUV_:
//             return "FAULT_OVUV";
//         case FAULT_OTUT_:
//             return "FAULT_OTUT";
//         case FAULT_COMM_:
//             return "FAULT_COMM";
//         case FAULT_OTP_:
//             return "FAULT_OTP";
//         case FAULT_COMP_ADC_:
//             return "FAULT_COMP_ADC";
//         case FAULT_PROT_:
//             return "FAULT_PROT";
//         default:
//             return "FAULT_UNKNOWN";
//     }
// }

// String getFaultString(FAULTS highLevelFault, int lowLevelFault) {
//     switch(highLevelFault) {
//         case FAULT_PWR_:
//             switch(lowLevelFault) {
//                 case FAULT_PWR1:
//                     return "FAULT_PWR1";
//                 case FAULT_PWR2:
//                     return "FAULT_PWR2";
//                 case FAULT_PWR3:
//                     return "FAULT_PWR3";
//                 default:
//                     return "FAULT_PWR_UNKNOWN";
//             }
//         case FAULT_SYS_:
//             switch (lowLevelFault) {
//                 case FAULT_SYS:
//                     return "FAULT_SYS";
//                 default:
//                     return "FAULT_SYS_UNKNOWN";
//             }
//         case FAULT_OVUV_:
//             switch(lowLevelFault) {
//                 case FAULT_OV1:
//                     return "FAULT_OV1";
//                 case FAULT_OV2:
//                     return "FAULT_OV2";
//                 case FAULT_UV1:
//                     return "FAULT_UV1";
//                 case FAULT_UV2:
//                     return "FAULT_UV2";
//                 default:
//                     return "FAULT_OVUV_UNKNOWN";
//             }
//         case FAULT_OTUT_:
//             switch(lowLevelFault) {
//                 case FAULT_OT:
//                     return "FAULT_OT";
//                 case FAULT_UT:
//                     return "FAULT_UT";
//                 default:
//                     return "FAULT_OTUT_UNKNOWN";
//             }
//         case FAULT_COMM_:
//             switch(lowLevelFault) {
//                 case FAULT_COMM1:
//                     return "FAULT_COMM1";
//                 case FAULT_COMM2:
//                     return "FAULT_COMM2";
//                 case FAULT_COMM3:
//                     return "FAULT_COMM3";
//                 default:
//                     return "FAULT_COMM_UNKNOWN";
//             }
//         case FAULT_OTP_:
//             switch(lowLevelFault) {
//                 case FAULT_OTP:
//                     return "FAULT_OTP";
//                 default:
//                     return "FAULT_OTP_UNKNOWN";
//             }
//         case FAULT_COMP_ADC_:
//             switch(lowLevelFault) {
//                 case FAULT_COMP_GPIO:
//                     return "FAULT_COMP_GPIO";
//                 case FAULT_COMP_VCCB1:
//                     return "FAULT_COMP_VCCB1";
//                 case FAULT_COMP_VCCB2:
//                     return "FAULT_COMP_VCCB2";
//                 case FAULT_COMP_VCOW1:
//                     return "FAULT_COMP_VCOW1";
//                 case FAULT_COMP_VCOW2:
//                     return "FAULT_COMP_VCOW2";
//                 case FAULT_COMP_CBOW1:
//                     return "FAULT_COMP_CBOW1";
//                 case FAULT_COMP_CBOW2:
//                     return "FAULT_COMP_CBOW2";
//                 case FAULT_COMP_CBFET1:
//                     return "FAULT_COMP_CBFET1";
//                 case FAULT_COMP_CBFET2:
//                     return "FAULT_COMP_CBFET2";
//                 case FAULT_COMP_MISC:
//                     return "FAULT_COMP_MISC";
//                 default:
//                     return "FAULT_COMP_ADC_UNKNOWN";
//             }
//         case FAULT_PROT_:
//             switch(lowLevelFault) {
//                 case FAULT_PROT1:
//                     return "FAULT_PROT1";
//                 case FAULT_PROT2:
//                     return "FAULT_PROT2";
//                 default:
//                     return "FAULT_PROT_UNKNOWN";
//             }
//         default:
//             return "FAULT_UNKNOWN";
//     }
// }

// int * getLowLevelFaultRegisters(FAULTS fault) {
//     static int faultRegisters[10];
//     memset(faultRegisters, 0, sizeof(faultRegisters));

//     switch(fault) {
//         case FAULT_PWR_:
//             faultRegisters[0] = FAULT_PWR1;
//             faultRegisters[1] = FAULT_PWR2;
//             faultRegisters[2] = FAULT_PWR3;
//             break;
//         case FAULT_SYS_:
//             faultRegisters[0] = FAULT_SYS;
//             break;
//         case FAULT_OVUV_:
//             faultRegisters[0] = FAULT_OV1;
//             faultRegisters[1] = FAULT_OV2;
//             faultRegisters[2] = FAULT_UV1;
//             faultRegisters[3] = FAULT_UV2;
//             break;
//         case FAULT_OTUT_:
//             faultRegisters[0] = FAULT_OT;
//             faultRegisters[1] = FAULT_UT;
//             break;
//         case FAULT_COMM_:
//             faultRegisters[0] = FAULT_COMM1;
//             faultRegisters[1] = FAULT_COMM2;
//             faultRegisters[2] = FAULT_COMM3;
//             break;
//         case FAULT_OTP_:
//             faultRegisters[0] = FAULT_OTP;
//             break;
//         case FAULT_COMP_ADC_:
//             faultRegisters[0] = FAULT_COMP_GPIO;
//             faultRegisters[1] = FAULT_COMP_VCCB1;
//             faultRegisters[2] = FAULT_COMP_VCCB2;
//             faultRegisters[3] = FAULT_COMP_VCOW1;
//             faultRegisters[4] = FAULT_COMP_VCOW2;
//             faultRegisters[5] = FAULT_COMP_CBOW1;
//             faultRegisters[6] = FAULT_COMP_CBOW2;
//             faultRegisters[7] = FAULT_COMP_CBFET1;
//             faultRegisters[8] = FAULT_COMP_CBFET2;
//             faultRegisters[9] = FAULT_COMP_MISC;
//             break;
//         case FAULT_PROT_:
//             faultRegisters[0] = FAULT_PROT1;
//             faultRegisters[1] = FAULT_PROT2;
//             break;
//         default:
//             break;
//     }


//     return faultRegisters;
// }


