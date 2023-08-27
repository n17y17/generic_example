#ifndef GENERIC_EXAMPLE_RP_PICO_TRANSMISSION_I2C_A_HPP_
#define GENERIC_EXAMPLE_RP_PICO_TRANSMISSION_I2C_A_HPP_

#include "Wire.h"

void SetupI2c(bool, uint32_t, std::initializer_list<uint8_t>);

void ReadI2c(bool, uint8_t, uint8_t*, size_t, uint8_t);
void ReadI2c_Direct(bool, uint8_t, uint8_t*, size_t);
void WriteI2c(bool, uint8_t, uint8_t*, size_t, uint8_t);
void WriteI2c_Direct(bool, uint8_t, uint8_t*, size_t);

// Arduino版では実装できていません
// void SetupI2cSlave(bool, uint32_t, std::initializer_list<uint8_t>, uint8_t, uint8_t*);

void SetupI2cSlave_Direct(bool, uint32_t, std::initializer_list<uint8_t>, uint8_t, uint8_t*, uint8_t*);

#endif