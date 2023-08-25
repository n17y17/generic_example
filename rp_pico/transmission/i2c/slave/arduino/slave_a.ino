#include "Wire.h"

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

const uint8_t kSdaGpio = 4;  // スレーブのSDAピンのGPIO番号
const uint8_t kSclGpio = 5;  // スレーブのSCLピンのGPIO番号
const uint8_t kSlaveAddr = 0x08;  // 自身のスレーブアドレス

// 割り込み処理の際に変更される変数を作成
static struct {
    uint8_t memory[256];  // メモリーを作成
    uint8_t memory_address;  // レジスタアドレス（配列のインデックス）
    // bool memory_address_written;  // 最初に受信するはずのレジスタアドレスをすでに受信したか
} Context;

static void I2c0DataReceive(int receive_len) {
    while (Wire.available() < receive_len) ;  // 要検証  // 全バイトが読み取り可能になるまで待機
    Context.memory_address = Wire.read();
    while (--receive_len) Context.memory[Context.memory_address++] = Wire.read();
}
static void I2c1DataReceive(int receive_len) {
    while (Wire1.available() < receive_len) ;  // 要検証  // 全バイトが読み取り可能になるまで待機
    Context.memory_address = Wire1.read();
    while (--receive_len) Context.memory[Context.memory_address++] = Wire1.read();
}

void setup() {
  Wire.begin(0x08);
  Wire.onReceive(I2c0DataReceive);
  Wire.onRequest();
}

void loop() {
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://garchiving.com/i2c-communication-with-arduino/
*/