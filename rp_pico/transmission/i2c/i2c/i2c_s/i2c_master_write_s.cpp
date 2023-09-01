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


uint8_t slave_addr = 0x08;
bool i2c_id = 0;
uint8_t scl_gpio = 8;
uint8_t sda_gpio = 3;
uint32_t i2c_freq = 30000;
uint8_t memory_addr = 5;
size_t input_data_bytes = 2;
uint8_t input_data[10];
uint8_t output_data[] = "abc";

I2c i2c0_master(i2c_id, scl_gpio, sda_gpio, i2c_freq);


I2c i2c1_slave(i2c_id, scl_gpio,  sda_gpio, i2c_freq, slave_addr);


void setup() {
    stdio_init_all();
    sleep_ms(1000);  // 要検証

    // SetupI2c(kI2cNum, kI2cBaudRate, {kSdaGpio, kSclGpio});  // Wireライブラリの初期化
}

void loop() {
    // uint8_t addr = 0x08;  // 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    // uint8_t data[] = "test\n";  // 送信するデータ
    // size_t len = 6;  // 何バイト(文字)書き込むか
    // uint8_t reg = 0x00;  // 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
    // WriteI2c(kI2cNum, addr, data, len, reg);  // スレーブへの書き込み
    // // レジスタアドレスを使用しない場合は WriteI2c_Direct 関数を使用してください

    // printf("write:%s\n", (char*)data);

    // uint8_t *ptr = output_data;
    uint8_t list[100];
    // char list2[] = {list, list};
    // uint8_t *list3 = static_cast<uint8_t*>(list2);

    i2c0_master.Read(slave_addr, list);
    i2c0_master.ReadMem(slave_addr, memory_addr, input_data);
    i2c0_master.Write(slave_addr, output_data);
    i2c0_master.WriteMem(slave_addr, memory_addr, output_data);

    i2c1_slave.DataReceivable();
    i2c1_slave.GetInputData(input_data, 4);
    i2c1_slave.SetOutputData(output_data);

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