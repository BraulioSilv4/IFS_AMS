#include <AMSMaster_Utils.hpp>

uint16_t readCellVoltage(char * response_frame) {
    uint16_t raw_data;
    ReadReg(DEVICE, VCELL16_HI, response_frame, 32, 0, FRMWRT_ALL_R);

    for(int nCell = 0; nCell < 16; nCell++) {
        // Read the two bytes of data from nCell registers
        // +4 because of headers probably?
        raw_data = (((response_frame[nCell * 2 + 4] & 0xFF) << 8) | (response_frame[nCell * 2 + 5] & 0xFF));

        raw_data = ((uint16_t)complement(raw_data) * 0.19073);
    }

    return raw_data;
}

float complement(uint16_t raw_data) {
    return -1*(~raw_data+1);
}
