#include "scXX_s.hpp"

int main()
{
    setup();
    while (true) loop();
}

void setup()
{
    try
    {
        I2c i2c0(i2c_id, scl_gpio, sda_gpio, baud_rate);
        Spi spi0(spi_id, tx_gpio, rx_gpio, baud_rate);

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void loop()
{
    try
    {
        switch (Phase)
        {
            case kWait:;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}