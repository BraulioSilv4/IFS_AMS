#include <AMSMaster_Utils.hpp>

BYTE response_frame[(16*2+6)*TOTALBOARDS]; 
uint16_t raw_data = 0;


const int DEVICE = 0;

void setup() {
    Wake79616();
    Serial1.begin(BAUDRATE);
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    AutoAddress();
    ADCSetup();
}

// the loop routine runs over and over again forever:
void loop() {
    // Read all registers from VCELL16-high to VCELL16-low
    ReadReg(DEVICE, VCELL16_HI, response_frame, 32, 0, FRMWRT_ALL_R);

    for(int nCell = 0; nCell < 16; nCell++) {
        // Read the two bytes of data from nCell registers
        // +4 because of headers probably?
        raw_data = (((response_frame[nCell * 2 + 4] & 0xFF) << 8) | (response_frame[nCell * 2 + 5] & 0xFF));

        raw_data = ((uint16_t)complement(raw_data) * 0.19073);

        Serial.print(raw_data);
    };
}



