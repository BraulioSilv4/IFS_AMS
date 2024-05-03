#include <AMSMaster_Utils.hpp>

float complement(uint16_t raw_data) {
    return -1*(~raw_data+1);
}
