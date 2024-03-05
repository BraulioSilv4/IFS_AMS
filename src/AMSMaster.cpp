#include <AMSMaster_Utils.hpp>

//ARRAYS (MUST place out of loop so not re-allocated every iteration)
BYTE response_frame[(16*2+6)*TOTALBOARDS]; //hold all 16 vcell*_hi/lo values

// the setup routine runs once when you press reset:
void setup() {
    Wake79616();
    Serial1.begin(BAUDRATE);
    delay(1);
    Serial.begin(ARDUINO_DUE_BAUDRATE);
    AutoAddress();
    ADCSetup();
}

// the loop routine runs over and over again forever:
void loop() {
    //*******************
    //READ CELL VOLTAGES
    //*******************

    //clear out the array so we know we have fresh data every loop
    memset(response_frame,0,sizeof(response_frame));

    //wait single round robin cycle time + reclocking delay for each device, every loop
    delayMicroseconds(192+5*TOTALBOARDS);

    //read all the values (HI/LO for each cell = 32 total)
    ReadReg(0, VCELL16_HI, response_frame, 16*2, 0, FRMWRT_ALL_R);

    /*
        * ***********************************************
        * NOTE: SOME COMPUTERS HAVE ISSUES TRANSMITTING
        * A LARGE AMOUNT OF DATA VIA PRINTF STATEMENTS.
        * THE FOLLOWING PRINTOUT OF THE RESPONSE DATA
        * IS NOT GUARANTEED TO WORK ON ALL SYSTEMS.
        * ***********************************************
    */

    //format and print the resultant response frame
    for(int currentBoard=0; currentBoard<TOTALBOARDS; currentBoard++)
    {
        //go through each byte in the current board (32 bytes = 16 cells * 2 bytes each)
        for(int i=0; i<32; i+=2)
        {
            //each board responds with 32 data bytes + 6 header bytes
            //so need to find the start of each board by doing that * currentBoard
            int boardByteStart = (16*2+6)*currentBoard;

            //convert the two individual bytes of each cell into a single 16 bit data item (by bit shifting)
            uint16 rawData = (response_frame[boardByteStart+i+4] << 8) | response_frame[boardByteStart+i+5];
            Serial.println(rawData);
            //cast as a signed 16 bit data item, and multiply by 190.73uV to get an actual voltage
            float cellVoltage = ((int16_t)rawData)*0.00019073;

            //print the voltages - it is (32-i)/2 because cells start from 16 down to 1
            //and there are 2 bytes per cell (i value is twice the cell number)
           // Serial.println(cellVoltage);
        }
    }
    //**********************
    //END READ CELL VOLTAGES
    //**********************
}



