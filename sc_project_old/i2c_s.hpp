#ifndef CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_I2C_S_HPP_
#define CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_I2C_S_HPP_

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

#include "communication_s.hpp"

// I2C通信を簡単に行うためのクラスです
class I2c : public Communication
{
    static bool kAlreadyUseI2c0;
    static bool kAlreadyUseI2c1;
    bool i2c_id_;
public:
    /*!
    \brief I2Cのセットアップ  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
    \param i2c_id i2c0かi2c1か
    \param scl_gpio i2cのSCLのGPIO番号
    \param sda_gpio i2cのSDAのGPIO番号
    \param i2c_freq i2cの転送速度
    */
    I2c(bool i2c_id, uint8_t scl_gpio, uint8_t sda_gpio, uint32_t i2c_freq);

    /*!
    \brief I2Cで受信 (マイコンなどから)
    \param slave_addr 通信先のデバイスのスレーブアドレス (どのデバイスからデータを読み込むか) 通常は8~119の間を使用する  7bit
    \param input_data 受信したデータを保存するための配列
    \param [input_data_bytes] 省略可  何バイト(文字)読み込むか  省略した場合はinput_dataの長さだけ読み取る
    */
    void Read(uint8_t slave_addr, uint8_t *input_data, std::size_t input_data_bytes);
    template<typename T, std::size_t SIZE> inline void Read(uint8_t slave_addr, T (&input_data)[SIZE]) {Read(slave_addr, (uint8_t*)input_data, SIZE);}

    /*!
    \brief I2Cで受信 (メモリから)
    \param slave_addr 通信先のデバイスのスレーブアドレス (どのデバイスからデータを読み込むか) 通常は8~119の間を使用する  7bit
    \param memory_addr 受信元のデバイスのメモリアドレス (スレーブ内のメモリの何番地からデータを読み込むか)
    \param input_data 受信したデータを保存するための配列
    \param [input_data_bytes] 省略可  何バイト(文字)読み込むか  省略した場合はinput_dataの長さだけ読み取る
    */
    void ReadMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *input_data, std::size_t input_data_bytes);
    template<typename T, std::size_t SIZE> inline void ReadMem(uint8_t slave_addr, uint8_t memory_addr, T (&input_data)[SIZE]) {ReadMem(slave_addr, memory_addr, (uint8_t*)input_data, SIZE);}

    /*!
    \brief I2Cで送信 (マイコンなどへ)
    \param slave_addr 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    \param output_data 送信するデータの配列
    \param [output_data_bytes] 省略可  何バイト(文字)書き込むか  省略した場合はoutput_dataの長さだけ書き込む
    */
    void Write(uint8_t slave_addr, uint8_t *output_data, std::size_t output_data_bytes);
    template<typename T, std::size_t SIZE> inline void Write(uint8_t slave_addr, T (&output_data)[SIZE]) {Write(slave_addr, (uint8_t*)output_data, SIZE);}

    /*!
    \brief I2Cで送信 (メモリへ)
    \param slave_addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    \param memory_addr : 送信先のデバイスのメモリアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
    \param output_data : 送信するデータの配列
    \param [output_data_bytes] 省略可  何バイト(文字)書き込むか  省略した場合はoutput_dataの長さだけ書き込む
    */
    void WriteMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *output_data, std::size_t output_data_bytes);
    template<typename T, std::size_t SIZE> inline void WriteMem(uint8_t slave_addr, uint8_t memory_addr, T (&output_data)[SIZE]) {WriteMem(slave_addr, memory_addr, (uint8_t*)output_data, SIZE);}
};
bool I2c::kAlreadyUseI2c0 = false;
bool I2c::kAlreadyUseI2c1 = false;

#endif  // CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_I2C_S_HPP_