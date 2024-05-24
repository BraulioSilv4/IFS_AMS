#include <AMSMaster_Utils.hpp>

uint16_t response_frame[(16*2+6)*TOTALBOARDS]; 
int c = 0;
int currBoard = 0;
uint16_t raw_data = 0;

void setup() {
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    // SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE0));    
    pinMode(nCS, OUTPUT);
    pinMode(MOSI, OUTPUT);
    pinMode(SPI_RDY, INPUT);

    digitalWrite(nCS, HIGH);
    digitalWrite(MOSI, HIGH);
    SPI.setClockDivider(SPI_CLOCK_DIV16);

    // Wake Sequence
    //two wakes in case the MCU had nCS and MOSI = 0 at start (which would put the device in shutdown) or in case the device was previously put in shutdown through a shutdown ping
    SpiWake79600(); //send wake ping to bridge device
    delayMicroseconds(3500); //wait tSU(WAKE_SHUT), at least 3.5ms
    SpiWake79600(); //send wake ping to bridge device
    delayMicroseconds(3500); //tSU(WAKE_SHUT), at least 3.5ms

    SPI.begin();
     
    SpiWriteReg(0, CONTROL1, 0x20, 1, FRMWRT_SGL_W); //send wake tone to stack devices
    delayMicroseconds((10000 + 600) * TOTALBOARDS); //wake tone duration is ~1.6ms per board + 10ms per board for each device to wake up from shutdown = 11.6ms per 616 board.
    
    // AutoAddress Sequence
    SpiAutoAddress();
    //delayMicroseconds(100);
    //Serial.println("################ REGISTERS ###################");
    SpiWriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_ALL_W);
    SpiReadReg(1, PARTID, response_frame, 1, 0, FRMWRT_STK_R);
    for (size_t i = 0; i < 7; i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    }

    // Activate cells
    SpiWriteReg(1, ACTIVE_CELL, 0x0A, 1, FRMWRT_STK_W);
    // Set the ADC run mode to continuous
    SpiWriteReg(1, ADC_CTRL1, 0x06, 1, FRMWRT_STK_W);
    // Wait the required round robin time
    delayMicroseconds(192 + (5 * TOTALBOARDS));

}


void loop() {
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
}
