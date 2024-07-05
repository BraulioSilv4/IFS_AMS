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
    if (comm_fault) {
        SerialUSB.println("Communication fault");
        shutdown();
    } else {
        if (millis() - itsBallingTime >= BAL_TIME && !comm_fault) {
            itsBallingTime = millis();
            
            if (Can1.available() > 0) {
                Can1.read(incomingFrame);
                double data = (incomingFrame.data.byte[0] << 8 | incomingFrame.data.byte[1]) / 1000;
                sendCellBalacingFrames(false);
                sendCellBalancingCurrentMillis = millis();
                balanceCells(data);
            }
        }

        if (millis() - sendCellBalancingCurrentMillis >= BAL_TIME && !comm_fault) {
            sendCellBalancingCurrentMillis = millis();
            sendCellBalacingFrames(true);
        }

        if (millis() - readFaultCurrentMillis >= READ_FAULT_INTERVAL && !comm_fault) {
            readFaultCurrentMillis = millis();
            readFaultSummary();
        }

        if (millis() - sendFaultCurrentMillis >= FAULT_UPDATE_INTERVAL && !comm_fault) {
            sendFaultCurrentMillis = millis();
            sendFaultFrames();
        }
        
        if (millis() - currentMillis >= UPDATE_INTERVAL && !comm_fault) {
            currentMillis = millis();

            CellVoltage cellData[(TOTALBOARDS-1)*ACTIVECHANNELS];
            readCells(ACTIVECHANNELS, VCELL16_HI, RESPONSE_FRAME_SIZE, cellData);

            CellVoltage GPIOdata[(TOTALBOARDS-1)*GPIOCHANNELS];
            readCells(GPIOCHANNELS, GPIO1_HI, 22, GPIOdata);

            CellTemperature cellTempData[(TOTALBOARDS-1)*GPIOCHANNELS];
            calculateCellTemperatures(GPIOdata, cellTempData, length(GPIOdata));
        
            sendVoltageFrames(cellData);
            sendTemperatureFrames(cellTempData);
            sendARStateFrame();
        }
    }
}