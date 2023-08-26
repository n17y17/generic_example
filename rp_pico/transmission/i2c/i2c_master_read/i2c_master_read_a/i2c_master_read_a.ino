// 未検証
#include "Wire.h"

/*
I2C通信を利用して，1000msに一回，テキストデータを受信
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
Wireライブラリの初期化  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
i2c_num : i2c0かi2c1か
i2c_baud_rate : i2cの転送速度
i2c_gpios : i2cのSDAとSCLのピン番号，{ }の中に入れて，{SDA,SCL,SDA,SCL,...}の順番で，使うものをすべて並べて書く
*/
void SetupI2c(bool i2c_num, uint32_t i2c_baud_rate, std::initializer_list<uint8_t> i2c_gpios) {
    int i = 0;
    if (i2c_num) {
        for (uint8_t i2c_gpio : i2c_gpios) {
            if ((++i) % 2) Wire1.setSDA(i2c_gpio);
            else Wire1.setSCL(i2c_gpio);
        }
        Wire1.begin();  // 引数のスレーブアドレスを省略したのでマスターとして接続
        Wire1.setClock(i2c_baud_rate);
    } else {
        for (uint8_t i2c_gpio : i2c_gpios) {
            if ((++i) % 2) Wire.setSDA(i2c_gpio);
            else Wire.setSCL(i2c_gpio);
        }
        Wire.begin();  // 引数のスレーブアドレスを省略したのでマスターとして接続
        Wire.setClock(i2c_baud_rate);
    }

    delay(10);  // 要検証
}

void setup() {
    Serial.begin(9600);

    SetupI2c(kI2cNum, kI2cBaudRate, {kSdaGpio, kSclGpio});
}

/*
スレーブからの読み込み
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
reg : 受信元のデバイスのレジスタアドレス (スレーブ内のメモリの何番地からデータを読み込むか)
data : 受信したデータを入れるための変数
len : 何バイト(文字)読み込むか
*/
inline void ReadI2c(bool i2c_num, uint8_t addr, uint8_t reg, uint8_t *data, size_t len) {
    if (i2c_num) {
        Wire1.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire1.write(reg);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        Wire1.endTransmission(false);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
        Wire1.requestFrom(addr, len, true);  // スレーブアドレスと要求するバイト数を指定し，データを受信する  第3引数は，停止信号を送りI2Cバスを開放するか
        while (Wire1.available() < len) ;  // 全バイトが読み取り可能になるまで待機
        while (len--) *(data++) = Wire.read();  // 受信したデータを読み取って保存
    } else {
        Wire.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire.write(reg);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        Wire.endTransmission(false);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
        Wire.requestFrom(addr, len, true);  // スレーブアドレスと要求するバイト数を指定し，データを受信する  第3引数は，停止信号を送りI2Cバスを開放するか
        while (Wire.available() < len) ;  // 全バイトが読み取り可能になるまで待機
        while (len--) *(data++) = Wire.read();  // 受信したデータを読み取って保存
    }

    delay(10);  // 要検証
}

void loop() {
    uint8_t addr = 0x08;  // 通信先のデバイスのスレーブアドレス (どのデバイスからデータを読み込むか) 通常は8~119の間を使用する  7bit
    uint8_t reg = 0x00;  // 受信元のデバイスのレジスタアドレス (スレーブ内のメモリの何番地からデータを読み込むか)
    uint8_t data[6] = {0};  // 受信したデータを入れるための変数
    size_t len = 6;  // 何バイト(文字)読み込むか
    ReadI2c(kI2cNum, addr, reg, data, len);

    Serial.println(sprintf("read:%s", (char*)data));

    delay(1000);
}

/*
このプログラムの作成にあたり以下を参考にしました
https://garchiving.com/i2c-communication-with-arduino/
https://garchiving.com/i2c-spi-communication-with-pico/
https://marycore.jp/prog/cpp/variadic-function/
*/