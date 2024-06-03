#include <AMSMaster_Utils.hpp>

uint16_t response_frame[RESPONSE_FRAME_SIZE];
uint16_t fault_response_frame[FAULT_FRAME_SIZE];

void setup() {
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    wakeSequence();
}

void loop() {
    std::vector<CellVoltage> cellData = readCells(DEVICE, TOTALBOARDS);
    std::vector<CellVoltage>::iterator it = cellData.begin();

    Serial.println("\n\nCell Data:");
    while (it != cellData.end()) {
        Serial.print("Channel: ");
        Serial.print(it->channel);
        Serial.print(" Voltage: ");
        Serial.print(it->voltage, 6);
        Serial.println(" V");
        it++;
    }

    std::vector<CellTemperature> cellTempData = calculateCellTemperatures(cellData);
    std::vector<CellTemperature>::iterator it2 = cellTempData.begin();

    Serial.println("\nCell Temp Data:");
    while (it2 != cellTempData.end()) {
        Serial.print("Channel: ");
        Serial.print(it2->channel);
        Serial.print(" Temp: ");
        Serial.print(it2->temperature, 2);
        Serial.println(" C");
        it2++;
    }

    std::vector<FAULTS> faultData = readFaults(DEVICE, TOTALBOARDS, FAULT_SUMMARY);
    //std::vector<FAULTS> faultData = {FAULT_COMP_ADC_, FAULT_OTUT_, FAULT_SYS_};  // Test faults
    std::vector<FAULTS>::iterator it1 = faultData.begin();
    
    
    Serial.println("\nFault Data:");
    if (faultData.empty()) {
        Serial.println("No faults detected.");
    } else {
        while (it1 != faultData.end()) {
            Serial.print("\nFault: ");
            Serial.println(getFaultSummaryString(*it1));

            std::vector<int> lowLevelFaultRegisters = getLowLevelFaultRegisters(*it1);
            std::vector<int>::iterator it2 = lowLevelFaultRegisters.begin();

            Serial.print("Low Level Fault Registers of Fault ");
            Serial.print(getFaultSummaryString(*it1));
            Serial.println(": ");
            while (it2 != lowLevelFaultRegisters.end()) {
                Serial.print(getFaultString(*it1, *it2));
                Serial.print(" Data: ");
                Serial.print(getFaultData(DEVICE, *it2), BIN);
                Serial.print("\n");
                it2++;
            }
            it1++;
        }
    }


    /*
    Serial.println("Die Temp Data:");
    while (it2 != dieTempData.end()) {
        Serial.print("Die: ");
        Serial.print(it2->die);
        Serial.print(" Temp: ");
        Serial.print(it2->temperature, 2);
        Serial.println(" Cº");
        it2++;
    }
    SpiReadReg(0, DIETEMP1_HI, response_frame, 2, 0, FRMWRT_STK_R);

    raw_data = (response_frame[4] << 8) | response_frame[5];

    float die_temp = raw_data * 0.00390625;
    Serial.print("Die Temp: ");
    Serial.print(die_temp, 2);
    Serial.println(" C");

    delay(1000);
    */
}

 /*
    // TEST
    SpiWriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_ALL_W);
    SpiReadReg(1, PARTID, response_frame, 1, 0, FRMWRT_STK_R);
    for (size_t i = 0; i < 7; i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    }

    double time = millis();
    if (time - c > 1000) {
        c = time;    
    
        int channel = 0;
        
        // Read the ADC values
        SpiReadReg(0, (VCELL16_HI+(16-ACTIVECHANNELS)*2), response_frame, ACTIVECHANNELS*2, 0, FRMWRT_STK_R);
        
        for(int currBoard = 0; currBoard < TOTALBOARDS-1; currBoard++) {

            for (channel = 0; channel < ACTIVECHANNELS*2; channel += 2) {
                int BoardStart = (ACTIVECHANNELS*2*+6) * currBoard;
                
                raw_data = (response_frame[channel + BoardStart + 4] << 8) | response_frame[channel + BoardStart + 5];
                Serial.println();
                Serial.print("Raw Data first part: ");
                Serial.println(response_frame[channel + BoardStart + 4] << 8, HEX);
                Serial.print("Raw Data second part: ");
                Serial.println(response_frame[channel + BoardStart + 5], HEX);
                Serial.print("Raw Data: ");
                Serial.println(raw_data, HEX);
                float cell_voltage = raw_data * 0.00019073;
            
                Serial.println();
                Serial.print("Board: ");
                Serial.print(currBoard);
                Serial.print(" Channel: ");
                Serial.print(channel/2 + 1);
                Serial.print(" Voltage: ");
                Serial.print(cell_voltage, 6);
                Serial.println(" V");
                
            }
        }

        SpiReadReg(0, DIETEMP1_HI, response_frame, 2, 0, FRMWRT_STK_R);

        raw_data = (response_frame[4] << 8) | response_frame[5];

        float die_temp = raw_data * 0.025;
        Serial.print("Die Temp: ");
        Serial.print(die_temp, 2);
        Serial.println(" C");
    }
    */
