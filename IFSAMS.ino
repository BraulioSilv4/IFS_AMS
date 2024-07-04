#include "AMSMaster_Utils.hpp"

// Timers
unsigned long currentMillis = 0;
unsigned long sendFaultCurrentMillis = 0;
unsigned long readFaultCurrentMillis = 0;

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    pinMode(CAN0_EN, OUTPUT);
    Can1.begin(CAN_BPS_500K, CAN0_EN);
    wakeSequence();
    initializeInternalStructures();
    currentMillis = millis();
}

void loop() { 
    SerialUSB.println("Looping");
    SerialUSB.println(comm_fault);
    if (comm_fault){
        SerialUSB.println("Communication fault detected. Restarting chips.");
        restartChips();
    } else {
        if (millis() - readFaultCurrentMillis >= READ_FAULT_INTERVAL) {
            readFaultCurrentMillis = millis();
            readFaultSummary();
        }

        if (millis() - sendFaultCurrentMillis >= FAULT_UPDATE_INTERVAL) {
            sendFaultCurrentMillis = millis();
            sendFaultFrames();
        }
        
        if (millis() - currentMillis >= UPDATE_INTERVAL) {
            currentMillis = millis();

            CellVoltage cellData[(TOTALBOARDS-1)*ACTIVECHANNELS];
            readCells(ACTIVECHANNELS, VCELL16_HI, RESPONSE_FRAME_SIZE, cellData);

            CellVoltage GPIOdata[(TOTALBOARDS-1)*GPIOCHANNELS];
            readCells(GPIOCHANNELS, GPIO1_HI, 22, GPIOdata);

            CellTemperature cellTempData[(TOTALBOARDS-1)*GPIOCHANNELS];
            calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));
        
            sendVoltageFrames(cellData);
            sendTemperatureFrames(cellTempData);
        }
    } 
}