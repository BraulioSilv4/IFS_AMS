#include "AMSMaster_Utils.hpp"

// Timers
unsigned long currentMillis = 0;
unsigned long sendFaultCurrentMillis = 0;
unsigned long readFaultCurrentMillis = 0;
unsigned long itsBallingTime = 0;
unsigned long sendCellBalancingCurrentMillis = 0;

CAN_FRAME incomingFrame;

void setup() {
    SerialUSB.begin(ARDUINO_DUE_BAUDRATE);
    pinMode(CAN0_EN, OUTPUT);
    Can1.begin(CAN_BPS_500K, CAN0_EN);
    Can1.watchFor(INCOMING_CB_FRAME_ID);
    wakeSequence();
    initializeInternalStructures();
    currentMillis = millis();
    sendFaultCurrentMillis = millis();
    readFaultCurrentMillis = millis();
    itsBallingTime = millis(); 
}

void loop() {
    if (millis() - itsBallingTime >= BAL_TIME) {
        itsBallingTime = millis();
        
        if (Can1.available() > 0) {
            Can1.read(incomingFrame);
            double data = (incomingFrame.data.byte[0] << 8 | incomingFrame.data.byte[1]) / 1000;
            sendCellBalacingFrames(false);
            sendCellBalancingCurrentMillis = millis();
            balanceCells(data);
        }
    }

    if (millis() - sendCellBalancingCurrentMillis >= BAL_TIME) {
        sendCellBalancingCurrentMillis = millis();
        sendCellBalacingFrames(true);
    }

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
        SerialUSB.println("Reading cell data:");
        for (int i = 0; i < length(cellData); i++) {
            SerialUSB.print("Board: ");
            SerialUSB.print(cellData[i].board);
            SerialUSB.print(" Channel: ");
            SerialUSB.print(cellData[i].channel);
            SerialUSB.print(" Voltage: ");
            SerialUSB.println(cellData[i].rawVoltage * 0.00019073);
        }

        CellVoltage GPIOdata[(TOTALBOARDS-1)*GPIOCHANNELS];
        readCells(GPIOCHANNELS, GPIO1_HI, 22, GPIOdata);
        // SerialUSB.println("Reading GPIO data:");
        // for (int i = 0; i < length(GPIOdata); i++) {
        //     SerialUSB.print("Board: ");
        //     SerialUSB.print(GPIOdata[i].board);
        //     SerialUSB.print(" Channel: ");
        //     SerialUSB.print(GPIOdata[i].channel);
        //     SerialUSB.print(" Voltage: ");
        //     SerialUSB.println(GPIOdata[i].rawVoltage * 0.00015259);
        // }

        CellTemperature cellTempData[(TOTALBOARDS-1)*GPIOCHANNELS];
        calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));
        // SerialUSB.println("Reading cell temperature data:");
        // for (int i = 0; i < length(cellTempData); i++) {
        //     SerialUSB.print("Channel: ");
        //     SerialUSB.print(cellTempData[i].channel);
        //     SerialUSB.print(" Temperature: ");
        //     SerialUSB.println(cellTempData[i].temperature);
        // }
    
        sendVoltageFrames(cellData);
        sendTemperatureFrames(cellTempData);
        sendARStateFrame();
    }
}