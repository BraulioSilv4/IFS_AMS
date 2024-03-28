#include <AMSMaster_Utils.hpp>

char response_frame[(16*2+6)*TOTALBOARDS]; 
uint16_t raw_data = 0;

void setup() {
    
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    Serial1.begin(BAUDRATE);
    Serial1.setTimeout(1000);

    /*
    pinMode(FAULT_PIN, OUTPUT);
    pinMode(NFAULT_PIN, INPUT);
    digitalWrite(FAULT_PIN, LOW);
    */

    Serial.print("Hello this the the AMS Code!\r\n");

    // Wake Sequence
    Wake79616();
    Serial.println("bq79616 Woken Up!\r\n");

    // AutoAddress Sequence
    // AutoAddress();
    //Serial.println("Autoaddress Completed");

    // set_registers();
}

// the loop routine runs over and over again forever:
void loop() {
    Serial.print("LOOP BEGIN\r\n");
    /*
    // Read Cell Voltages
    raw_data = readCellVoltage(response_frame);
    Serial.print("Raw Data: ");
    Serial.print(raw_data);
    */

    delay(3000);
    WriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_SGL_W);
    delay(2000);
    ReadReg(DEVICE, COMM_TIMEOUT_CONF, response_frame, 4, 0, FRMWRT_ALL_R);

    for (size_t i = 0; i < sizeof(response_frame); i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    }     
}
