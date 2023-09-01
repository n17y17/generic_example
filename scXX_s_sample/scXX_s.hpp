#ifndef SCXX_SCXX_S_HPP_
#define SCXX_SCXX_S_HPP_

#include <stdio.h>
#include <deque>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

enum class Phase
{
    kWait,
    kDrop,
    kLongDistance,
    kShortDistance,
    KGoaled,
    kExcept
}

void setup();
void wait_phase();
void falling_phase();
void long_distanse_phase();
void short_distance_phase();
void goaled_phase();

#endif  // SCXX_SCXX_S_HPP_