// 未検証
#include <stdio.h>
#include <initializer_list>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

/*
I2C通信を利用して，1000msに一回，特定のテキストデータを送信
*/

/*
I2C通信は1台のマスターと複数台のスレーブの間の通信です．
通信はデータ通信線(SDA)と，クロック信号線(CLK)により行います．

マスターはスレーブに自由にデータを送信できますが，
スレーブはマスターからの指示を受信したときのみ送信できます．(マスターが送るクロック信号を基準にして通信する)

各スレーブには，スレーブアドレス(マスターが指示を送るための，デバイスごとのアドレス)が割り当てられています．
デバイスの各メモリにはレジスタアドレス(メモリ内の番地を表すアドレス)が割り当てられています．

マスターは各通信の最初に，これから通信したいスレーブのレジスタアドレスを送信します．(通信相手のメモリの何番地に書き込む/読み込むかを指定する)
(このとき，スレーブアドレスも一緒に送り誰宛の通信かも分かるようにしているが，あまり意識しなくてもOK)
その後に，マスターが送信しようとした場合，スレーブは初めに送られてきたレジスタアドレスの場所にデータを書き込みます．
　　　　　マスターが受信しようとした場合，スレーブは初めに送られてきたレジスタアドレスの場所のデータを送信します．
*/

const bool kI2cNum = 0;  // i2c0かi2c1か
const uint32_t kI2cBaudRate = 100 * 1000;  // 通信速度  Hz  通常は400kHz以下を使う
const uint8_t kSdaGpio = 4;  // SDAピンのGPIO番号
const uint8_t kSclGpio = 5;  // SCLピンのGPIO番号

/*
I2Cの初期化  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
i2c_num : i2c0かi2c1か
i2c_baud_rate : i2cの転送速度
i2c_gpios : i2cのSDAとSCLのピン番号，{ }の中に入れて，{SDA,SCL,SDA,SCL,...}の順番で，使うものをすべて並べて書く
*/
void SetupI2c(bool i2c_num, uint32_t i2c_baud_rate, std::initializer_list<uint8_t> i2c_gpios) {
    i2c_init((i2c_num ? i2c1 : i2c0), i2c_baud_rate);

    for (uint8_t i2c_gpio : i2c_gpios) {
        gpio_set_function(i2c_gpio, GPIO_FUNC_I2C);  // GPIOピンの有効化
        gpio_pull_up(i2c_gpio);
    }

    sleep_ms(10);  // 要検証
}

void setup() {
    stdio_init_all();
    sleep_ms(1000);  // 要検証

    SetupI2c(kI2cNum, kI2cBaudRate, {kSdaGpio, kSclGpio});
}

/*
スレーブへの書き込み
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
reg : 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
data : 送信するデータ  配列の先頭へのポインタ
len : 何バイト(文字)書き込むか
*/
inline void WriteI2c(bool i2c_num, uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    if (i2c_num) {
        i2c_write_blocking(i2c1, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c1, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c0, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

void loop() {
    uint8_t addr = 0x08;  // 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    uint8_t reg = 0x00;  // 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
    uint8_t data[] = "test\n";  // 送信するデータ
    size_t len = 6;  // 何バイト(文字)書き込むか
    WriteI2c(kI2cNum, addr, reg, data, len);

    printf("write:%s\n", (char*)data);

    sleep_ms(1000);
}

int main() {
    setup();

    while(true) loop();
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://omoroya.com/arduino-extra-edition-06/
https://marycore.jp/prog/cpp/variadic-function/
*/