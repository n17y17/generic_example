#ifndef GENERIC_EXAMPLE_RP_PICO_TRANSMISSION_I2C_S_HPP_
#define GENERIC_EXAMPLE_RP_PICO_TRANSMISSION_I2C_S_HPP_

#include <stdio.h>
#include <deque>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

/*
-----I2C通信とは-----
I2C通信は1台のマスターと複数台のスレーブの間の通信です．
通信はデータ通信線(SDA)と，クロック信号線(CLK)により行います．
！マスターの二つのピンだけで，複数のセンサに繋げます！
master   slave1   slave2  ...
 SDA <==> SDA <==> SDA <==> ...
 SCL ===> SCL ===> SCL ===> ...

マスターはスレーブに自由にデータを送信できますが，
スレーブはマスターからの指示を受信したときのみ送信できます．(マスターが送るクロック信号を基準にして通信する)

各スレーブには，スレーブアドレス(マスターが指示を送るための，デバイスごとのアドレス)が割り当てられています．
デバイスの各メモリにはメモリアドレス(=レジスタアドレス)(メモリ内の番地を表すアドレス)が割り当てられています．

マスターは各通信の最初に，これから通信したいスレーブのメモリアドレスを送信します．(通信相手のメモリの何番地に書き込む/読み込むかを指定する)
その後に，マスターが送信しようとした場合，スレーブは初めに送られてきたメモリアドレスの場所にデータを書き込みます．
　　　　　マスターが受信しようとした場合，スレーブは初めに送られてきたメモリアドレスの場所のデータを送信します．
--------------------
*/

/**
 * I2C通信を簡単に行うためのライブラリです
 * pico-SDKを使用しています
*/

class I2c {
  private:
    static bool kAlreadyUseI2c0;
    static bool kAlreadyUseI2c1;
    bool i2c_id_;
  public:

    //-----マスター用-----

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




    // -----スレーブ用-----

    /*!
    \brief I2Cをスレーブとしてセットアップ (送受信にメモリアドレスを使用しない)
    \param i2c_id i2c0かi2c1か
    \param scl_gpio i2cのSCLのGPIO番号
    \param sda_gpio i2cのSDAのGPIO番号
    \param i2c_freq i2cの転送速度
    \param slave_addr 自身のスレーブアドレス
    */
    I2c(bool i2c_id, uint8_t scl_gpio, uint8_t sda_gpio, uint32_t i2c_freq, uint8_t slave_addr);

    //! \brief すでに受信した，読み取り可能なデータのバイト数を返す
    std::size_t DataReceivable();

    /*!
    \brief マスターから受信し，一時保存してあったたデータを読み取る
    データの読み取りが完了したら，一時データは削除されます
    \param input_data 受信したデータを保存するための配列
    \param [input_data_bytes] 省略可  何バイト(文字)読み込むか  省略した場合はinput_dataの長さだけ読み取る
    */
    std::size_t GetInputData(uint8_t *input_data, std::size_t input_data_bytes);
    template<typename T, std::size_t SIZE> inline std::size_t GetInputData(T (&input_data)[SIZE]) {return GetInputData((uint8_t*)input_data, SIZE);}

    /*!
    \brief マスターに送信するためのデータをセットする
    \param output_data 送信するデータの配列
    \param [output_data_bytes] 省略可  何バイト(文字)書き込むか  省略した場合はoutput_dataの長さだけ書き込む
    */
    void SetOutputData(uint8_t *output_data, std::size_t output_data_bytes);
    template<typename T, std::size_t SIZE> inline void SetOutputData(T (&output_data)[SIZE]) {SetOutputData((uint8_t*)output_data, SIZE);}
};

#endif