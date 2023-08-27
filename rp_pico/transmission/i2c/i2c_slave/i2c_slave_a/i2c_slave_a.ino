// 未検証
#include "i2c_a.hpp"

/*
これはスレーブ用のプログラムです
I2C通信を利用して，マスターから送られてきたデータをメモリーに保存
マスターのリクエストに応じてメモリーのデータを送信
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
const uint8_t kSdaGpio = 4;  // スレーブのSDAピンのGPIO番号
const uint8_t kSclGpio = 5;  // スレーブのSCLピンのGPIO番号
const uint8_t kI2cSlaveAddr = 0x08;  // 自身のスレーブアドレス

// 入力データを入れるための変数を作成
uint8_t kI2cInputData[256];  // 絶対にグローバル変数を使用！！

// 出力データを作成
uint8_t kI2cOutputData[] = "hello World";  // 絶対にグローバル変数を使用！！

void setup() {
    Serial.begin(9600);
    delay(1000);  // 要検証

    // I2Cをスレーブとしてセットアップする
    SetupI2cSlave_Direct(kI2cNum, kI2cBaudRate, {kSdaGpio, kSclGpio}, kI2cSlaveAddr, kI2cInputData, kI2cOutputData);
    // マスター側のプログラムは WriteI2c_Direct, ReadI2c_Direct を使ってください．
    // WriteI2c, Readi2c を使うと，正常に通信できません
}

void loop() {
    // 入力データkI2cInputDataは入力を受けたときに勝手に更新されます
    // ここで，入力データを読み取り，利用してください
    // 出力データの更新も行えます
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://garchiving.com/i2c-communication-with-arduino/
*/