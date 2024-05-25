#ifndef AMSMASTER_UTILS_HPP_
#define AMSMASTER_UTILS_HPP_

#include <map>
#include <string>
#include <iostream>

// Check and undefine min and max macros
#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <algorithm> // Ensure this is included after undefining macros

#include <bq79616.hpp>

#define ARDUINO_DUE_BAUDRATE    9600
#define DEVICE                  0

float complement(uint16_t raw_data);
void wakeSequence();
std::map<int, float> readCells(int device, int totalBoards);

#endif // AMSMASTER_UTILS_HPP_
//EOF
