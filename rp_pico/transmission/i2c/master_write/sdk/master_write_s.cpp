#include "pico/stdlib.h"
#include "hardware/i2c.h"

/*
I2C通信は1台のマスターと複数台のスレーブの間の通信です．
通信はクロック信号線(CLK)と，データ通信線(SDA)により行います．

マスターはスレーブに自由にデータを送信できますが，
スレーブはマスターからの指示を受信したときのみ送信できます．(マスターが送るクロック信号を基準にする)
1本のデータ信号線ですべての通信を行うため，受信はスレーブも好きな時に行えます．

各スレーブには，マスターが指示を送るための個別のアドレスが割り当てられています．
*/

i2c_inst_t *kI2c = i2c0;  //i2c0かi2c1か
const uint kBaudRate = 9600;  //通信速度  Hz
const uint kSdaGpio = 4;  //SDAピンのGPIO番号
const uint kSclGpio = 5;  //SCLピンのGPIO番号

void setup() {
    //I2Cアクセスを初期化
    i2c_init(kI2c, GPIO_FUNC_I2C);  //デフォルトはマスター

    //I2CのGPIOをセットする
    gpio_set_function(kSdaGpio, GPIO_FUNC_I2C);
    gpio_set_function(kSclGpio, GPIO_FUNC_I2C);
    gpio_pull_up(kSdaGpio);
    gpio_pull_up(kSclGpio);
}

int main() {
    setup();

    uint8_t addr = 0x08;  //送信先のスレーブのアドレス  通常は8~119の間を使用する  7bit
    uint8_t src[] = "test\n";  //送信するデータ
    size_t len = sizeof(src);  //何バイト(文字)送信するか
    bool nostop = true;  //次の通信まで他のデバイスに割り込ませないか
    i2c_write_blocking(kI2c, addr, &src[0], len, nostop);  //送信  3番目の引数は送信するデータの配列の先頭へのポインタ
    sleep_ms(10);
}

/*
このプログラムの作成にあたり以下のサイトを参考にしました．
https://stemship.com/arduino-beginner-i2c/
*/