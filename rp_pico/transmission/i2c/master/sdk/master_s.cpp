// 未検証
#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/stdlib.h"

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

i2c_inst_t *kI2c = i2c0;  // i2c0かi2c1か
const uint kBaudRate = 9600;  // 通信速度  Hz
const uint kSdaGpio = 4;  // SDAピンのGPIO番号
const uint kSclGpio = 5;  // SCLピンのGPIO番号

void setup() {
    // I2Cアクセスを初期化
    i2c_init(kI2c, GPIO_FUNC_I2C);  // デフォルトはマスター

    // I2CのGPIOをセットする
    gpio_set_function(kSdaGpio, GPIO_FUNC_I2C);
    gpio_set_function(kSclGpio, GPIO_FUNC_I2C);
    gpio_pull_up(kSdaGpio);
    gpio_pull_up(kSclGpio);
}

int main() {
    setup();

    //---------------
    // 受信
    uint8_t addr = 0x08;  // これから通信したいデバイスのスレーブアドレス  通常は8~119の間を使用する  7bit
    uint8_t reg = 0x00;  // 受信元のデバイスのレジスタアドレス (スレーブ内のメモリの何番地からデータを読み込むか)
    uint8_t dst[5];  // 受信した値を保存するための変数
    size_t len = sizeof(dst);  // 何バイト(文字)受信するか
    bool nostop = false;  // 停止信号を送らず，次の通信まで他のデバイスに割り込ませないか
    i2c_write_blocking(kI2c, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
    i2c_read_blocking(kI2c, addr, &dst[0], len, nostop);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ
    sleep_ms(10);
    printf("%s\n", dst);
    //---------------


    //---------------
    // 送信
    addr = 0x08;  // これから通信したいデバイスのスレーブアドレス  通常は8~119の間を使用する．  7bit
    reg = 0x00;  // 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
    uint8_t src[] = "test";  // 送信するデータ
    len = sizeof(src);  // 何バイト(文字)送信するか
    nostop = true;  // 停止信号を送らず，次の通信まで他のデバイスに割り込ませないか
    i2c_write_blocking(kI2c, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
    i2c_write_blocking(kI2c, addr, &src[0], len, nostop);  // データを送信  3番目の引数は送信するデータの配列の先頭へのポインタ
    sleep_ms(10);
    printf("send\n");
    //---------------
}

/*
このプログラムの作成にあたり以下のサイトを参考にしました．
https://stemship.com/arduino-beginner-i2c/
*/