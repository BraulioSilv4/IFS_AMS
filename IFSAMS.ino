#include "AMSMaster_Utils.hpp"

unsigned long currentMillis = 0;
unsigned long faultCurrentMillis = 0;

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    
    pinMode(CAN0_EN, OUTPUT);
    Can1.begin(CAN_BPS_500K, CAN0_EN);

    wakeSequence();

    initializeInternalStructures();
    
    currentMillis = millis();
}

void loop() { 

    
    if (millis() - faultCurrentMillis >= FAULT_UPDATE_INTERVAL) {
        faultCurrentMillis = millis();
        BOARD_FAULT_SUMMARY boardFaultSummary[TOTALBOARDS-1];
        readFaultSummary(boardFaultSummary);
        sendFaultFrames(boardFaultSummary);
    }
    
    if (millis() - currentMillis >= UPDATE_INTERVAL) {
        currentMillis = millis();

        SerialUSB.println("Reading cell data");
        CellVoltage cellData[(TOTALBOARDS-1)*ACTIVECHANNELS];
        readCells(ACTIVECHANNELS, VCELL16_HI, RESPONSE_FRAME_SIZE, cellData);
        SerialUSB.println("Ended reading cell data");

        CellVoltage GPIOdata[(TOTALBOARDS-1)*GPIOCHANNELS];
        readCells(GPIOCHANNELS, GPIO1_HI, 22, GPIOdata);

        CellTemperature cellTempData[(TOTALBOARDS-1)*GPIOCHANNELS];
        calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));
        SerialUSB.println("CELL TEMP DATA:");
        
        sendVoltageFrames(cellData);
        sendTemperatureFrames(cellTempData);
    }
}