#ifndef CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_COMMUNICATION_S_HPP_
#define CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_COMMUNICATION_S_HPP_

#include <stdio.h>

#include "pico/stdlib.h"

#include "exception_s.hpp"

/*
通信方式
*/
enum CommunicationMethod
{
    kI2c,
    kSpi,
    kUart
    // kDegital
};

/*
これは抽象クラスです
通信に関するクラスの継承元クラスとして利用します
ここで "= 0" と定義されている関数を継承先のクラスでオーバーライド(再定義して上書き)しないとコンパイルエラーになります
*/
class Communication
{
    virtual void Read() = 0;  // マイコンなどから読み込み
    virtual void Write() = 0;  // マイコンなどに書き込み
    virtual void ReadMem() = 0;  // センサなどのメモリから読み込み
    virtual void WriteMem() = 0;  // センサなどのメモリへ書き込み
};

#endif  // CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_COMMUNICATION_S_HPP_