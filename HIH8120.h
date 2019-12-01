/*
 * File:   HIH8120.h
 * Author: Radu Grecu, Dominik Kovacs
 *
 * Created on November 4, 2019, 10:30 AM
 */

#ifndef HIH8120_H
#define HIH8120_H

#include <utility>

#include "I2CDevice.h"

namespace exploringBB {

class HIH8120 : protected I2CDevice {
public:
    float humidity;
    float temperature;
    HIH8120(unsigned int bus, unsigned int slave_address);
    void read_sensor();
};

#endif /* HIH8120_H */

}