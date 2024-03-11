/*
*
*/
#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <bq79616.hpp>

#define ARDUINO_DUE_BAUDRATE    115200

void ADCSetup();

float complement(uint16_t raw_data);

#endif // AMSMASTER_UTILS_HPP_
//EOF
