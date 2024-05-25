#include <AMSMaster_Utils.hpp>

void setup() {
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    wakeSequence();
}

void loop() {
    std::map<int, float> cellData = readCells(DEVICE, TOTALBOARDS);
    std::map<int, float>::iterator it = cellData.begin();

    while (it != cellData.end()) {
        Serial.print("Channel: ");
        Serial.print(it->first);
        Serial.print(" Voltage: ");
        Serial.print(it->second, 6);
        Serial.println(" V");
        it++;
    }
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