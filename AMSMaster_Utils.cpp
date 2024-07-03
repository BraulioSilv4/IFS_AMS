#include "AMSMaster_Utils.hpp"

const FAULT_INFO FaultInfo[TOTALFAULT_BIT] = {
    {FAULT_PWR_, {FAULT_PWR1, FAULT_PWR2, FAULT_PWR3}, 3, FAULT_RST1, 0x01},
    {FAULT_SYS_, {FAULT_SYS}, 1, FAULT_RST1, 0x02},
    {FAULT_OVUV_, {FAULT_OV1, FAULT_OV2, FAULT_UV1, FAULT_UV2}, 4, FAULT_RST1, 0x18},
    {FAULT_OTUT_, {FAULT_OT, FAULT_UT}, 2, FAULT_RST1, 0x60},
    {FAULT_COMM_, {FAULT_COMM1, FAULT_COMM2, FAULT_COMM3}, 3, FAULT_RST2, 0x1F},
    {FAULT_OTP_, {FAULT_OTP}, 1, FAULT_RST2, 0x60},
    {FAULT_COMP_ADC_, {FAULT_COMP_GPIO, FAULT_COMP_VCCB1, FAULT_COMP_VCCB2, FAULT_COMP_VCOW1, FAULT_COMP_VCOW2, FAULT_COMP_CBOW1, FAULT_COMP_CBOW2, FAULT_COMP_CBFET1, FAULT_COMP_CBFET2, FAULT_COMP_MISC}, 10, FAULT_RST1, 0x04},
    {FAULT_PROT_, {FAULT_PROT1, FAULT_PROT2}, 2, FAULT_RST1, 0x80}
};

BOARD_FAULTS_DATA boardsFaultsData[TOTALBOARDS-1];

void initializeInternalStructures() {
    for (size_t i = 0; i < TOTALBOARDS-1; i++) {
        boardsFaultsData[i].boardID = i+1;
        for (size_t j = 0; j < TOTALFAULT_BIT; j++) {
            boardsFaultsData[i].faults[j].fault = static_cast<FAULT>(j);
            boardsFaultsData[i].faults[j].timeout = 0;
            boardsFaultsData[i].faults[j].hasTimeoutStarted = false;
        }
    }
}

void sendWakeTone() {
    SpiWriteReg(0, CONTROL1, 0x20, 1, FRMWRT_SGL_W); //send wake tone to stack devices
    delayMicroseconds((10000 + 600) * TOTALBOARDS); //wake tone duration is ~1.6ms per board + 10ms per board for each device to wake up from shutdown = 11.6ms per 616 board.
}

void setPins() {
    pinMode(nCS, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SPI_RDY, INPUT);
    pinMode(SHUTDOWN1, OUTPUT);
    pinMode(SHUTDOWN2, OUTPUT);

    digitalWrite(nCS, HIGH);
    digitalWrite(MOSI, HIGH);
    digitalWrite(SHUTDOWN1, HIGH);
    digitalWrite(SHUTDOWN2, HIGH);
}

void setRegisters() {
    SpiWriteReg(DEVICE, CONTROL2, 0x01, 1, FRMWRT_ALL_W); // enable TSREF

    // CONFIGURE GPIOS as temp inputs
    SpiWriteReg(DEVICE, GPIO_CONF1, 0x12, 1, FRMWRT_STK_W); // GPIO1 and 2 as temp inputs
    SpiWriteReg(DEVICE, GPIO_CONF2, 0x12, 1, FRMWRT_STK_W); // GPIO3 and 4 as temp inputs
    SpiWriteReg(DEVICE, GPIO_CONF3, 0x12, 1, FRMWRT_STK_W); // GPIO5 and 6 as temp inputs
    SpiWriteReg(DEVICE, GPIO_CONF4, 0x12, 1, FRMWRT_STK_W); // GPIO7 and 8 as temp inputs

    //SpiWriteReg(0, OTUT_THRESH, 0xDA, 1, FRMWRT_ALL_W); // Sets OV thresh to 80% and UT thresh to 20% to meet rules

    SpiWriteReg(DEVICE, OV_THRESH, 0x22, 1, FRMWRT_STK_W); // Sets Over voltage protection to 4.175V
    SpiWriteReg(DEVICE, UV_THRESH, 0x24, 1, FRMWRT_STK_W); // Sets Under voltage protection to 3.0V
    SpiWriteReg(DEVICE, OVUV_CTRL, 0x05, 1, FRMWRT_STK_W); // Sets voltage controls   
   
    // Activate cells
    SpiWriteReg(1, ACTIVE_CELL, 0x0A, 1, FRMWRT_ALL_W);
    // LPF_ON - LPF = 9ms
    SpiWriteReg(0, ADC_CONF1, 0x04, 1, FRMWRT_ALL_W);
    // Wait the required round robin time
    delayMicroseconds(192 + (5 * TOTALBOARDS));
    // Sets communication timeout to 2 s
    SpiWriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_STK_W); 

    // START THE MAIN ADC
    SpiWriteReg(0, ADC_CTRL1, 0x1E, 1, FRMWRT_STK_W); // continuous run and MAIN_GO and LPF_VCELL_EN and CS_DR = 1ms
    SpiWriteReg(0, ADC_CTRL2, 0x00, 1, FRMWRT_STK_W); // continuous run and MAIN_GO and LPF_VCELL_EN and CS_DR = 1ms
    SpiWriteReg(0, ADC_CTRL3, 0x00, 1, FRMWRT_STK_W); // continuous run and MAIN_GO and LPF_VCELL_EN and CS_DR = 1ms
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
    ResetAllFaults(DEVICE, FRMWRT_ALL_W);
}

void shutdown() {
    digitalWrite(SHUTDOWN1, LOW);
    digitalWrite(SHUTDOWN2, LOW);
}

CellVoltage readCell(int device, int reg, int channel) {
    CellVoltage cellData;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];

    SpiReadReg(device, reg, response_frame, 2, 0, FRMWRT_SGL_R);

    uint16_t raw_data = (response_frame[4] << 8) | response_frame[5];
    cellData.board = device;
    cellData.channel = channel;
    cellData.rawVoltage = raw_data;

    return cellData;
}

void readCells(int channels, int reg, int frameSize, CellVoltage cellData[]) {
    uint16_t raw_data = 0;
    uint16_t response_frame[frameSize];
    
    for(int currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {
        SpiReadReg(currBoard+1, reg, response_frame, channels*2, 0, FRMWRT_SGL_R);
        SerialUSB.println("Reading cells from board:");
        SerialUSB.println(currBoard+1);
        for (int channel = 0; channel < channels*2; channel += 2) {
            SerialUSB.println("Channel:");
            SerialUSB.println(channel/2 + 1);
            raw_data = (response_frame[channel + 4] << 8) | response_frame[channel + 5];
            int idx = currBoard*channels + channel/2;
            cellData[idx].board = currBoard+1;
            cellData[idx].channel = channel/2 + 1;
            cellData[idx].rawVoltage = raw_data;
        }
    }
}

void readGPIOS(int channels, int reg, int frameSize, CellVoltage cellData[]) {
    uint16_t raw_data = 0;
    uint16_t response_frame[RESPONSE_FRAME_SIZE];

    SpiReadReg(DEVICE, reg, response_frame, channels*2, 0, FRMWRT_STK_R);

    for(int currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {

        for (int channel = 0; channel < channels*2; channel += 2) {
            int boardStart = (channels*2*+6) * currBoard;

            raw_data = (response_frame[channel + boardStart + 4] << 8) | response_frame[channel + boardStart + 5];
            int idx = currBoard*channels + channel/2;
            cellData[idx].channel = channel/2 + 1;
            cellData[idx].rawVoltage = raw_data;
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
        float cellVoltage = cellData[i].rawVoltage * 0.00015259;
        double temp = Interpolation::Linear(xValues, yValues, 33, cellVoltage, true);
        cellTempData[i].channel = cellData[i].channel;
        cellTempData[i].temperature = static_cast<float>(temp);
    }
}

void resetClearedFaults(BOARD_FAULT_SUMMARY boardFaultSummary) {
    for (size_t i = 0; i < TOTALFAULT_BIT; i++) {
        FAULT fault = static_cast<FAULT>(i);
        int boardIdx = boardFaultSummary.board - 1;
        uint8_t faultSummary = boardFaultSummary.faultSummary;
        if (!(faultSummary & (1 << i))) {
            if (boardsFaultsData[boardIdx].faults[i].hasTimeoutStarted) {
                boardsFaultsData[boardIdx].faults[i].hasTimeoutStarted = false;
            }
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
                myCANFrame.data.byte[3] = static_cast<uint8_t>(GPIOtemp) & 0xFF;
                myCANFrame.data.byte[4] = (static_cast<uint8_t>(GPIOtemp) >> 8) & 0xFF;
                myCANFrame.data.byte[5] = (static_cast<uint8_t>(GPIOtemp) >> 16) & 0xFF;;
                myCANFrame.data.byte[6] = (static_cast<uint8_t>(GPIOtemp) >> 24) & 0xFF;

                Can1.sendFrame(myCANFrame);
            }
        }
    }
}

void sendVoltageFaultFrame(int device, int reg, uint16_t fault_response_frame[], int faultType) {
    SpiReadReg(device, reg, fault_response_frame, 2, 0, FRMWRT_SGL_R);
    uint16_t v = fault_response_frame[4] << 8 | fault_response_frame[5];

    if (v != 0) {
        int baseReg = VCELL1_HI;
        for (size_t i = 0; i < ACTIVECHANNELS; i++) {
            if (v & (1 << i)) {
                CellVoltage cellData = readCell(device, baseReg - i*2, i);

                CAN_FRAME myCANFrame;
                myCANFrame.id = VOLTAGE_FAULT_ID;
                myCANFrame.length = 5;
                myCANFrame.data.byte[0] = device;
                myCANFrame.data.byte[1] = ACTIVECHANNELS - i;
                myCANFrame.data.byte[2] = faultType;
                myCANFrame.data.byte[3] = cellData.rawVoltage & 0xFF;
                myCANFrame.data.byte[4] = (cellData.rawVoltage >> 8) & 0xFF;

                Can1.sendFrame(myCANFrame);    
            }
        }
    }
}

void sendFaultFrame(int device, FAULT fault, uint16_t fault_response_frame[]) {
    for (size_t i = 0; i < FaultInfo[fault].size; i++) {
        int reg = FaultInfo[fault].lowLevelFaults[i];
        SpiReadReg(device, reg, fault_response_frame, 1, 0, FRMWRT_SGL_R);
        uint16_t f = fault_response_frame[4];
        if (f != 0) {
            CAN_FRAME myCANFrame;
            myCANFrame.id = FAULT_ID;
            myCANFrame.length = 4;
            myCANFrame.data.byte[0] = device;
            myCANFrame.data.byte[1] = fault;
            myCANFrame.data.byte[2] = i;
            myCANFrame.data.byte[3] = f;
            
            Can1.sendFrame(myCANFrame);
        }
    }
}

void sendFaultFrames(BOARD_FAULT_SUMMARY boardsFaultSummary[]) {
    uint16_t fault_response_frame[FAULT_FRAME_SIZE];
    for(size_t i = 0; i < TOTALBOARDS-1; i++) {
        uint8_t faultSummary = boardsFaultSummary[i].faultSummary;
        int board = boardsFaultSummary[i].board;
        if (faultSummary != 0) {
            for (size_t j = 0; j < TOTALFAULT_BIT; j++){
                FAULT fault = static_cast<FAULT>(j);
                if (faultSummary & (1 << j)) {
                    if (!boardsFaultsData[i].faults[j].hasTimeoutStarted) {
                        boardsFaultsData[i].faults[j].timeout = millis();
                        boardsFaultsData[i].faults[j].hasTimeoutStarted = true;
                    } 

                    if (millis() - boardsFaultsData[i].faults[j].timeout <= FAULT_TIMEOUT){
                        SpiWriteReg(board, FaultInfo[fault].rstReg, FaultInfo[fault].rstVal, 1, FRMWRT_SGL_W); // reset fault
                    } else {
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
                        //shutdown();
                    }
                }
            }
        }
    }
}

void sendVoltageFrames(CellVoltage cellData[]) {
    for (size_t currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {
        for (size_t channel = 0; channel < ACTIVECHANNELS; channel += 4) {
            int id = BASE_VOLTAGE_FRAME_ID + currBoard*4 + channel/4;
            uint16_t firstVoltage = cellData[currBoard*ACTIVECHANNELS + channel].rawVoltage;
            uint16_t secondVoltage = cellData[currBoard*ACTIVECHANNELS + channel + 1].rawVoltage;
            uint16_t thirdVoltage = cellData[currBoard*ACTIVECHANNELS + channel + 2].rawVoltage;
            uint16_t fourthVoltage = cellData[currBoard*ACTIVECHANNELS + channel + 3].rawVoltage;
            CAN_FRAME myCANFrame;
            myCANFrame.id = id;
            myCANFrame.length = 8;
            myCANFrame.data.byte[0] = firstVoltage & 0xFF;
            myCANFrame.data.byte[1] = (firstVoltage >> 8) & 0xFF;
            myCANFrame.data.byte[2] = secondVoltage & 0xFF;
            myCANFrame.data.byte[3] = (secondVoltage >> 8) & 0xFF;
            myCANFrame.data.byte[4] = thirdVoltage & 0xFF;
            myCANFrame.data.byte[5] = (thirdVoltage >> 8) & 0xFF;
            myCANFrame.data.byte[6] = fourthVoltage & 0xFF;
            myCANFrame.data.byte[7] = (fourthVoltage >> 8) & 0xFF;
            
            Can1.sendFrame(myCANFrame);
        }
    }
}

void sendTemperatureFrames(CellTemperature cellTempData[]) {
    for (size_t currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {
        for (size_t channel = 0; channel < GPIOCHANNELS; channel += 2) {
            int id = BASE_TEMPERATURE_FRAME_ID + currBoard*2 + channel/2;
            uint32_t firstTemp = cellTempData[currBoard*GPIOCHANNELS + channel].temperature * TEMPERATURE_SCALING_FACTOR;
            uint32_t secondTemp = cellTempData[currBoard*GPIOCHANNELS + channel + 1].temperature * TEMPERATURE_SCALING_FACTOR;
            CAN_FRAME myCANFrame;
            myCANFrame.id = id;
            myCANFrame.length = 8;
            myCANFrame.data.byte[0] = firstTemp & 0xFF;
            myCANFrame.data.byte[1] = (firstTemp >> 8) & 0xFF;
            myCANFrame.data.byte[2] = (firstTemp >> 16) & 0xFF;
            myCANFrame.data.byte[3] = (firstTemp >> 24) & 0xFF;
            myCANFrame.data.byte[4] = secondTemp & 0xFF;
            myCANFrame.data.byte[5] = (secondTemp >> 8) & 0xFF;
            myCANFrame.data.byte[6] = (secondTemp >> 16) & 0xFF;
            myCANFrame.data.byte[7] = (secondTemp >> 24) & 0xFF;

            Can1.sendFrame(myCANFrame);
        }
    }
}
    



float complement(uint16_t raw_data) {
     return -1*(~raw_data+1);
}

