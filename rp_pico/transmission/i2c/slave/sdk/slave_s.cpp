// 未検証
#include <stdio.h>

#include "hardware/i2c.h"
#include "pico/i2c_slave.h"
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
const uint kSdaGpio = 4;  // スレーブのSDAピンのGPIO番号
const uint kSclGpio = 5;  // スレーブのSCLピンのGPIO番号
const uint kSlaveAddr = 0x08;  // 自身のスレーブアドレス

// 割り込み処理の際に変更される変数を作成
static struct {
    uint8_t memory[256];  // メモリーを作成
    uint8_t memory_address;  // レジスタアドレス（配列のインデックス）
    bool memory_address_written;  // 最初に受信するはずのレジスタアドレスをすでに受信したか
} Context;

// マスターからの受信があったとき，この関数が割り込み処理で実行される
static void I2cSlaveHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
      case I2C_SLAVE_RECEIVE: {  // マスターがデータを書き込んだとき
        if (!Context.memory_address_written) {
            // 最初にメモリアドレスが書き込まれるので，記録する
            Context.memory_address = i2c_read_byte_raw(i2c);
            Context.memory_address_written = true;
        } else {
            // マスターがデータを送信したい場合
            Context.memory[Context.memory_address] = i2c_read_byte_raw(i2c);  // 初めに記録したレジスタアドレスのメモリに，受信した内容を記録する
            Context.memory_address++;
        }
        break;
      }
      case I2C_SLAVE_REQUEST: {  // マスターがデータを要求しているとき
        // マスターがデータを受信したい場合
        i2c_write_byte_raw(i2c, Context.memory[Context.memory_address]);  // 初めに記録したレジスタアドレスのメモリの内容を送信する．
        Context.memory_address++;
        break;
      }
      case I2C_SLAVE_FINISH: {  // マスターが停止／再起動の信号を送信したとき
        // メモリアドレスをリセットする
        Context.memory_address_written = false;
        break;
      }
      default: {
        break;
      }
    }
}

void setup() {
    // I2CのGPIOをセットする
    gpio_init(kSdaGpio);
    gpio_init(kSclGpio);
    gpio_set_function(kSdaGpio, GPIO_FUNC_I2C);
    gpio_set_function(kSclGpio, GPIO_FUNC_I2C);
    gpio_pull_up(kSdaGpio);
    gpio_pull_up(kSclGpio);

    // I2Cアクセスを初期化
    i2c_init(kI2c, GPIO_FUNC_I2C);  //デフォルトはマスター
    
    // I2Cをスレーブに設定したうえで，送受信時の割り込み処理を設定
    i2c_slave_init(kI2c, kSlaveAddr, &I2cSlaveHandler);
}

int main() {
    setup();
}

/*
このプログラムの作成にあたり以下のサイトを参考にしました．
https://stemship.com/arduino-beginner-i2c/
pico_sdkのサンプルコードslave_mem_i2c.c
*/