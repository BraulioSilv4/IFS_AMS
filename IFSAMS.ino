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
    SerialUSB.println("Reading fault data");
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
        readGPIOS(GPIOCHANNELS, GPIO1_HI, 54, GPIOdata);
        SerialUSB.println("GPIO DATA:");
        for (int i = 0; i < length(GPIOdata); i++) {
            SerialUSB.print("Board: ");
            SerialUSB.print(GPIOdata[i].board);
            SerialUSB.print(" Channel: ");
            SerialUSB.print(GPIOdata[i].channel);
            SerialUSB.print(" Voltage: ");
            SerialUSB.println(GPIOdata[i].rawVoltage * 0.00015259);
        }

        CellTemperature cellTempData[(TOTALBOARDS-1)*GPIOCHANNELS];
        calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));
        // SerialUSB.println("CELL TEMP DATA:");
        // for (int i = 0; i < length(cellTempData); i++) {
        //     SerialUSB.print("Channel: ");
        //     SerialUSB.print(cellTempData[i].channel);
        //     SerialUSB.print(" Temp: ");
        //     SerialUSB.println(cellTempData[i].temperature);
        // }
        sendVoltageFrames(cellData);
        sendTemperatureFrames(cellTempData);
    }
}