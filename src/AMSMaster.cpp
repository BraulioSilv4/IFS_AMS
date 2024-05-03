#include <AMSMaster_Utils.hpp>

uint16_t response_frame[(16*2+6)*TOTALBOARDS]; 
int c = 0;
int currBoard = 0;

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
    Serial.println("################ REGISTERS ###################");
    SpiWriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_ALL_W);
    SpiReadReg(1, COMM_TIMEOUT_CONF, response_frame, 2, 0, FRMWRT_SGL_R);
    
    
    /*
    //Serial.begin(ARDUINO_DUE_BAUDRATE);
   

    /*
    pinMode(FAULT_PIN, OUTPUT);
    pinMode(NFAULT_PIN, INPUT);
    digitalWrite(FAULT_PIN, LOW);
    */
    
    //Serial.print("Hello this the the AMS Code!\r\n");

    
    /*
    // Configure GPIO
    PIO_Configure(
        g_APinDescription[PINS_USART1].pPort,
        g_APinDescription[PINS_USART1].ulPinType,
        g_APinDescription[PINS_USART1].ulPin,
        g_APinDescription[PINS_USART1].ulPinConfiguration
    );
    

    //WriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_SGL_W);

    // AutoAddress Sequence
    AutoAddress();
    delayMicroseconds(4000);
    WriteReg(0, FAULT_MSK2, 0x40, 1, FRMWRT_ALL_W); //MASK CUST_CRC SO CONFIG CHANGES DON'T FLAG A FAULT
    ResetAllFaults(0, FRMWRT_ALL_W); //CLEAR ALL FAULTS
    Serial.println("Autoaddress Completed");

    /*
    //set up the main ADC
    WriteReg(0, ACTIVE_CELL, 0x0A, 1, FRMWRT_ALL_W);    //set all cells to active
    WriteReg(0, ADC_CONF1, 0x02, 1, FRMWRT_ALL_W);      //26Hz LPF_Vcell (38ms average)
    WriteReg(0, ADC_CTRL1, 0x0E, 1, FRMWRT_ALL_W);      //continuous run, LPF enabled and MAIN_GO
    delayMicroseconds(38000+5*TOTALBOARDS);                       //initial delay to allow LPF to average for 38ms (26Hz LPF setting used)
    

    // Set Registers
    set_registers();
    Serial.println("Registers Set");
    */
}

// the loop routine runs over and over again forever:
void loop() {
    /*

    c = 0;
    currBoard = 0;
    delayMicroseconds(192+5*TOTALBOARDS);
    SpiReadReg(0, VCELL16_HI+(16-ACTIVECHANNELS)*2, response_frame, ACTIVECHANNELS*2, 0, FRMWRT_STK_R);

    /*
        * ***********************************************
        * NOTE: SOME COMPUTERS HAVE ISSUES RECEIVING
        * A LARGE AMOUNT OF DATA VIA printf STATEMENTS.
        * THE FOLLOWING PRINTOUT OF THE RESPONSE DATA
        * IS NOT GUARANTEED TO WORK ON ALL SYSTEMS.
        * ***********************************************
    

    Serial.print("\n"); //start with a newline to add some extra spacing between each loop
    //only read/print the base device's data if there is no bridge device
    for(currBoard=0; currBoard<( BRIDGEDEVICE==1 ? TOTALBOARDS-1 : TOTALBOARDS); currBoard++)
    {
        Serial.print("BOARD %d:\t");
        Serial.print(TOTALBOARDS-currBoard-1);
        //print the data from each active channel (2 bytes each channel)
        for(c=0; c<(ACTIVECHANNELS*2); c+=2)
        {
            int boardByteStart = (ACTIVECHANNELS*2+6)*currBoard;
            uint16_t rawData = (response_frame[boardByteStart+c+4] << 8) | response_frame[boardByteStart+c+5];
            float cellVoltage = rawData*0.00019073; //rawData*VLSB_ADC
            Serial.print("%f\t");
            Serial.print(cellVoltage);
        }
        Serial.print("\n"); //newline per board
    }
    */

    //INITIALIZE BQ79616-Q1 STACK
    /*
    digitalWrite(nCS, LOW);
    SPI.transfer(0x90);
    SPI.transfer(0x00);
    SPI.transfer(0x03);
    SPI.transfer(0x09);
    SPI.transfer(0x20);
    SPI.transfer(0x13);
    SPI.transfer(0x95);
    digitalWrite(nCS, HIGH);
    */

    /*
    Serial.println("\n\nLOOP BEGIN\r\n");

    //clear out the array so we know we have fresh data every loop
    memset(response_frame,0,sizeof(response_frame));

    ReadReg(DEVICE, CUST_CRC_RSLT_HI, response_frame, 1, 0, FRMWRT_SGL_R);
    Serial.print("\nCUST_CRC_RSLT_HI: ");
    for (size_t i = 0; i < sizeof(response_frame); i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    }
    /*
    

    //wait single round robin cycle time + reclocking delay for each device, every loop
    delayMicroseconds(192+5*TOTALBOARDS);

    
    /*
    ReadReg(DEVICE, COMM_TIMEOUT_CONF, response_frame, 1, 0, FRMWRT_SGL_R);
    Serial.print("\nRESPONSE FRAME: ");
    for (size_t i = 0; i < sizeof(response_frame); i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    } 
    */

    //WriteReg(DEVICE, COMM_TIMEOUT_CONF, 0x0A, 1, FRMWRT_SGL_W);
    /*
    ReadReg(DEVICE, COMM_TIMEOUT_CONF, response_frame, 4, 0, FRMWRT_SGL_R);

    Serial.print("\nRESPONSE FRAME: ");
    for (size_t i = 0; i < sizeof(response_frame); i++) {
        Serial.print(response_frame[i], HEX);
        Serial.print(" ");
    } 
    
    
    /*
    /*
    // Read Cell Voltages
    raw_data = readCellVoltage(response_frame);
    Serial.print("Raw Data: ");
    Serial.print(raw_data);
  
    
 
    */
}
