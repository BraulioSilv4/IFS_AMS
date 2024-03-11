#include <AMSMaster_Utils.hpp>

//set up the main ADC
void ADCSetup() {
    WriteReg(0, ACTIVE_CELL, 0x0A, 1, FRMWRT_ALL_W);    //set all cells to active
    WriteReg(0, ADC_CONF1, 0x02, 1, FRMWRT_ALL_W);      //26Hz LPF_Vcell (38ms average)
    WriteReg(0, ADC_CTRL1, 0x0E, 1, FRMWRT_ALL_W);      //continuous run, LPF enabled and MAIN_GO
    delayMicroseconds(38000+5*TOTALBOARDS);             //initial delay to allow LPF to average for 38ms (26Hz LPF setting used)
}

float complement(uint16_t raw_data) {
    return -1*(~raw_data+1);
}