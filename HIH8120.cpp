/*
 * File:   HIH8120.cpp
 * Author: Radu Grecu, Dominik Kovacs
 * 
 * Created on November 4, 2019, 10:30 AM
 */

#include <cmath>
#include <unistd.h>
#include "HIH8120.h"

namespace exploringBB {

HIH8120::HIH8120(unsigned int bus, unsigned int slave_address) : I2CDevice(bus, slave_address) {}

void HIH8120::read_sensor() {

    int reading_h, reading_t;

    // request to initiate measurement
    this->write(0);

    // wait at least 50 ms, datasheet indicates that measurement cycle is usually 36.65 ms
    usleep(50 * 1000);

    // request measured data
    unsigned char* reading = this->readDevice(4);

    // parse data
    reading_h = (reading[0] & 0x3fff);
    reading_h <<= 8;
    reading_h |= reading[1];

    this->humidity = reading_h / (pow(2.0, 14.0) - 2) * 100;

    reading_t = reading[2];
    reading_t <<= 6;
    reading_t |= reading[3] >> 2;

    this->temperature = reading_t / (pow(2.0, 14.0) - 2) * 165 - 40;

}

}
