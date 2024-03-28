/*
*
*/
#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <bq79616.hpp>

#define ARDUINO_DUE_BAUDRATE    9600
#define DEVICE                  0

uint16_t readCellVoltage(uint8_t cell, uint8_t *response_frame);

float complement(uint16_t raw_data);

#endif // AMSMASTER_UTILS_HPP_
//EOF
