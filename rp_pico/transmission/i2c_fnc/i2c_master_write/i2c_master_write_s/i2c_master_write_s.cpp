// 未検証
#include "i2c_s.hpp"

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

void setup() {
    stdio_init_all();
    sleep_ms(1000);  // 要検証

    SetupI2c(kI2cNum, kI2cBaudRate, {kSdaGpio, kSclGpio});  // Wireライブラリの初期化
}

void loop() {
    uint8_t addr = 0x08;  // 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    uint8_t data[] = "test\n";  // 送信するデータ
    size_t len = 6;  // 何バイト(文字)書き込むか
    uint8_t reg = 0x00;  // 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
    WriteI2c(kI2cNum, addr, data, len, reg);  // スレーブへの書き込み
    // レジスタアドレスを使用しない場合は WriteI2c_Direct 関数を使用してください

    printf("write:%s\n", (char*)data);

    sleep_ms(1000);
}

int main() {
    setup();

    while (true) loop();
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://omoroya.com/arduino-extra-edition-06/
https://marycore.jp/prog/cpp/variadic-function/
*/