#include "AMSMaster_Utils.hpp"

uint16_t fault_response_frame[FAULT_FRAME_SIZE];
long current_time;

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    CAN.begin(CAN_BPS_500K);
    wakeSequence();
}

void loop() {
    current_time = millis();
    CellVoltage cellData[(TOTALBOARDS-1)*ACTIVECHANNELS];
    readCells(DEVICE, TOTALBOARDS, ACTIVECHANNELS, VCELL16_HI, cellData);

    SerialUSB.println("\n\nCell Data:");
    for (int i = 0; i < length(cellData); i++) {
        SerialUSB.print("Channel: ");
        SerialUSB.print(cellData[i].channel);
        SerialUSB.print(" Voltage: ");
        SerialUSB.print(cellData[i].voltage, 6);
        SerialUSB.println(" V");
    }

    CellVoltage GPIOdata[(TOTALBOARDS-1)*GPIOCHANNELS];
    readCells(DEVICE, TOTALBOARDS, GPIOCHANNELS, GPIO8_HI, GPIOdata);

    SerialUSB.println("\n\nGPIO Data:");
    for (int i = 0; i < length(GPIOdata); i++) {
        SerialUSB.print("Channel: ");
        SerialUSB.print(GPIOdata[i].channel);
        SerialUSB.print(" Voltage: ");
        SerialUSB.print(GPIOdata[i].voltage, 6);
        SerialUSB.println(" V");
    }

    CellTemperature cellTempData[length(GPIOdata)];
    calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));

    SerialUSB.println("\nCell Temp Data:");
    for (int i = 0; i < length(cellTempData); i++) {
        SerialUSB.print("Channel: ");
        SerialUSB.print(cellTempData[i].channel);
        SerialUSB.print(" Temp: ");
        SerialUSB.print(cellTempData[i].temperature, 2);
        SerialUSB.println(" C");
    }

    BOARD_FAULT_SUMMARY boardFaultSummary[TOTALBOARDS-1];
    readFaultSummary(boardFaultSummary);
    for (size_t i = 0; i < TOTALBOARDS-1; i++) {
        SerialUSB.print("\nBoard: ");
        SerialUSB.print(boardFaultSummary[i].board);
        SerialUSB.print(" Fault Summary: ");
        SerialUSB.println(boardFaultSummary[i].faultSummary, BIN);
    }
    SerialUSB.println("\n\n");
    
    sendFaultFrames(boardFaultSummary);
}