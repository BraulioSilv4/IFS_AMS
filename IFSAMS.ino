#include "AMSMaster_Utils.hpp"

uint16_t response_frame[RESPONSE_FRAME_SIZE];
uint16_t fault_response_frame[FAULT_FRAME_SIZE];

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    SerialUSB.println("Starting AMSMaster...");
    wakeSequence();
}

void loop() {
    pinMode(48, OUTPUT);
    digitalWrite(48, HIGH);
    SerialUSB.println("Bomboclaat");
    CellVoltage cellData[(TOTALBOARDS-1)*ACTIVECHANNELS];
    readCells(DEVICE, TOTALBOARDS, cellData);

    // SerialUSB.println("\n\nCell Data:");
    // for (int i = 0; i < length(cellData); i++) {
    //     SerialUSB.print("Channel: ");
    //     SerialUSB.print(cellData[i].channel);
    //     SerialUSB.print(" Voltage: ");
    //     SerialUSB.print(cellData[i].voltage, 6);
    //     SerialUSB.println(" V");
    // }
    /*
    CellTemperature CellTempData[]
    CellTemperature * cellTempData = calculateCellTemperatures(cellData);

    SerialUSB.println("\nCell Temp Data:");
    for (int i = 0; i < length(cellTempData); i++) {
        SerialUSB.print("Channel: ");
        SerialUSB.print(cellTempData[i].channel);
        SerialUSB.print(" Temp: ");
        SerialUSB.print(cellTempData[i].temperature, 2);
        SerialUSB.println(" C");
    }

    FAULTS * faultData = readFaults(DEVICE, TOTALBOARDS, FAULT_SUMMARY);
    int faultDataSize = length(faultData);

    SerialUSB.println("\nFault Data:");
    if (faultDataSize == 0){
        SerialUSB.println("No faults detected.");
    } else {
        for (int i = 0; i < faultDataSize; i++) {
            SerialUSB.print("\nFault: ");
            SerialUSB.println(getFaultSummaryString(faultData[i]));

            int * lowLevelFaultRegisters = getLowLevelFaultRegisters(faultData[i]);
            
            SerialUSB.print("Low Level Fault Registers of Fault ");
            SerialUSB.print(getFaultSummaryString(faultData[i]));
            SerialUSB.println(": ");
            for (int i = 0; i < length(lowLevelFaultRegisters); i++) {
                SerialUSB.print(getFaultString(faultData[i], lowLevelFaultRegisters[i]));
                SerialUSB.print(" Data: ");
                SerialUSB.print(getFaultData(DEVICE, lowLevelFaultRegisters[i]), BIN);
                SerialUSB.print("\n");
            }
        }
    }
    */
}