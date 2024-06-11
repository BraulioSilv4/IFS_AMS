#include "AMSMaster_Utils.hpp"

uint16_t response_frame[RESPONSE_FRAME_SIZE];
uint16_t fault_response_frame[FAULT_FRAME_SIZE];

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    wakeSequence();
}

void loop() {
    CellVoltage * cellData = readCells(DEVICE, TOTALBOARDS);

    SerialUSB.println("\n\nCell Data:");
    for (int i = 0; i < lenght(cellData); i++) {
        Serial.print("Channel: ");
        Serial.print(cellData[i].channel);
        Serial.print(" Voltage: ");
        Serial.print(cellData[i].voltage, 6);
        Serial.println(" V");
    }

    CellTemperature * cellTempData = calculateCellTemperatures(cellData);

    Serial.println("\nCell Temp Data:");
    for (int i = 0; i < lenght(cellTempData); i++) {
        Serial.print("Channel: ");
        Serial.print(cellTempData[i].channel);
        Serial.print(" Temp: ");
        Serial.print(cellTempData[i].temperature, 2);
        Serial.println(" C");
    }

    FAULTS * faultData = readFaults(DEVICE, TOTALBOARDS, FAULT_SUMMARY);
    int faultDataSize = lenght(faultData);

    Serial.println("\nFault Data:");
    if (faultDataSize == 0){
        Serial.println("No faults detected.");
    } else {
        for (int i = 0; i < faultDataSize; i++) {
            Serial.print("\nFault: ");
            Serial.println(getFaultSummaryString(faultData[i]));

            int * lowLevelFaultRegisters = getLowLevelFaultRegisters(faultData[i]);
            
            Serial.print("Low Level Fault Registers of Fault ");
            Serial.print(getFaultSummaryString(faultData[i]));
            Serial.println(": ");
            for (int i = 0; i < lenght(lowLevelFaultRegisters); i++) {
                Serial.print(getFaultString(faultData[i], lowLevelFaultRegisters[i]));
                Serial.print(" Data: ");
                Serial.print(getFaultData(DEVICE, lowLevelFaultRegisters[i]), BIN);
                Serial.print("\n");

            }
        }
    }
}