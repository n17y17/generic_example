// 未検証
#include "i2c_s.hpp"

/*
I2C通信を簡単に行うための関数です
*/

/*
-----I2C通信とは-----
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

// -----ここから，マスター用の関数-----

/*
I2Cのマスターとしての初期化  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
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

/*
スレーブからの読み込み (レジスタアドレスを指定する)
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
data : 受信したデータを入れるための変数
len : 何バイト(文字)読み込むか
reg : 受信元のデバイスのレジスタアドレス (スレーブ内のメモリの何番地からデータを読み込むか)
*/
void ReadI2c(bool i2c_num, uint8_t addr, uint8_t *data, size_t len, uint8_t reg) {
    if (i2c_num) {
        i2c_write_blocking(i2c1, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        i2c_read_blocking(i2c1, addr, data, len, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        i2c_read_blocking(i2c0, addr, data, len, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

/*
スレーブからの読み込み (レジスタアドレスを指定しない)
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
data : 受信したデータを入れるための変数
len : 何バイト(文字)読み込むか
*/
void ReadI2c_Direct(bool i2c_num, uint8_t addr, uint8_t *data, size_t len) {
    if (i2c_num) {
        i2c_read_blocking(i2c1, addr, data, len, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_read_blocking(i2c0, addr, data, len, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

/*
スレーブへの書き込み (レジスタアドレスを指定する)
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
data : 送信するデータ  配列の先頭へのポインタ
len : 何バイト(文字)書き込むか
reg : 送信先のデバイスのレジスタアドレス (スレーブ内のメモリの何番地にデータを書き込むか)
*/
void WriteI2c(bool i2c_num, uint8_t addr, uint8_t *data, size_t len, uint8_t reg) {
    if (i2c_num) {
        i2c_write_blocking(i2c1, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c1, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, addr, &reg, 1, true);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c0, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

/*
スレーブへの書き込み (レジスタアドレスを指定しない)
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
data : 送信するデータ  配列の先頭へのポインタ
len : 何バイト(文字)書き込むか
*/
void WriteI2c_Direct(bool i2c_num, uint8_t addr, uint8_t *data, size_t len) {
    if (i2c_num) {
        i2c_write_blocking(i2c1, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, addr, data, len, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

// ----ここから，スレーブ用の関数-----

// 変数の受け渡し用
static inline uint8_t *set_memory(bool i2c_num, uint8_t *memory = nullptr) {
    if (i2c_num) {
        static uint8_t *kMemory1 = memory;
        return kMemory1;
    } else {
        static uint8_t *kMemory0 = memory;
        return kMemory0;
    }
    
}
static inline uint8_t *set_input_data(bool i2c_num, uint8_t *input_data = nullptr) {
    if (i2c_num) {
        static uint8_t *kInputData1 = input_data;
        return kInputData1;
    } else {
        static uint8_t *kInputData0 = input_data;
        return kInputData0;
    }
}
static inline uint8_t *set_output_data(bool i2c_num, uint8_t *output_data = nullptr) {
    if (i2c_num) {
        static uint8_t *kOutputData1 = output_data;
        return kOutputData1;
    } else {
        static uint8_t *kOutputData0 = output_data;
        return kOutputData0;
    }
}

/*
マスターからの受信があったとき，この関数が割り込み処理で実行される
(レジスタアドレスを受信する)
*/
static void I2cSlaveHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kI2cSlaveMemory = set_memory(i2c == i2c1);
    static uint8_t kI2cSlaveMemoryAddr = 0x00;  // レジスタアドレス（配列のインデックス）
    static bool kI2cSlaveMemoryAddrWritten = false;  // 最初に受信するはずのレジスタアドレスをすでに受信したか

    switch (event) {
      case I2C_SLAVE_RECEIVE: {  // マスターがデータを書き込んだとき
        if (!kI2cSlaveMemoryAddrWritten) {
            // 最初にレジスタアドレスが書き込まれるので，記録する
            kI2cSlaveMemoryAddr = i2c_read_byte_raw(i2c);
            kI2cSlaveMemoryAddrWritten = true;
        } else {
            // マスターがデータを送信したい場合
            *(kI2cSlaveMemory + kI2cSlaveMemoryAddr) = i2c_read_byte_raw(i2c);  // 初めに記録したレジスタアドレスのメモリに，受信した内容を記録する
            printf(">%c", *(kI2cSlaveMemory + kI2cSlaveMemoryAddr));
            ++kI2cSlaveMemoryAddr;
        }
        break;
      }
      case I2C_SLAVE_REQUEST: {  // マスターがデータを要求しているとき
        // マスターがデータを受信したい場合
        i2c_write_byte_raw(i2c, *(kI2cSlaveMemory + kI2cSlaveMemoryAddr));  // 初めに記録したレジスタアドレスのメモリの内容を送信する．
        printf("<%c", *(kI2cSlaveMemory + kI2cSlaveMemoryAddr));
        ++kI2cSlaveMemoryAddr;
        break;
      }
      case I2C_SLAVE_FINISH: {  // マスターが停止／再起動の信号を送信したとき
        // レジスタアドレスをリセットする
        kI2cSlaveMemoryAddrWritten = false;
        break;
      }
      default: {
        break;
      }
    }
}

/*
マスターからの受信があったとき，この関数が割り込み処理で実行される
(レジスタアドレスを受信しない)
*/
static void I2cSlaveHandler_Direct(i2c_inst_t *i2c, i2c_slave_event_t event) {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kInputData = set_input_data(i2c == i2c1);
    static uint8_t *kOutputData = set_output_data(i2c == i2c1);
    static uint8_t kI2cSlaveMemoryAddr = 0x00;  // レジスタアドレスは0から始まるものとして考える
    bool read = false;
    switch (event) {
      case I2C_SLAVE_RECEIVE: {  // マスターがデータを書き込んだとき
        kInputData[kI2cSlaveMemoryAddr] = i2c_read_byte_raw(i2c);  // 初めに記録したレジスタアドレスのメモリに，受信した内容を記録する
        read = true;
        printf(">%c", kInputData[kI2cSlaveMemoryAddr]);
        ++kI2cSlaveMemoryAddr;
        break;
      }
      case I2C_SLAVE_REQUEST: {  // マスターがデータを要求しているとき
        i2c_write_byte_raw(i2c, kOutputData[kI2cSlaveMemoryAddr]);  // 初めに記録したレジスタアドレスのメモリの内容を送信する．
        printf("<%c", kOutputData[kI2cSlaveMemoryAddr]);
        ++kI2cSlaveMemoryAddr;
        break;
      }
      case I2C_SLAVE_FINISH: {  // マスターが停止／再起動の信号を送信したとき
        if(read) for (int i = kI2cSlaveMemoryAddr, len = sizeof(kInputData) / sizeof(*kInputData); i < len; ++i) kInputData[i] = 0;  // InputDataの最後は0で埋める
        kI2cSlaveMemoryAddr = 0x00;  // レジスタアドレスを0にリセットする
        break;
      }
      default: {
        break;
      }
    }
}

/*
I2Cをスレーブとしてセットアップする (送受信にレジスタアドレスを使用する)
変数でメモリーを作成し同一のメモリーに対して，受信したレジスタアドレスの場所へ書き込み，受信したレジスタアドレスの場所から出力を行います．
i2c_num : i2c0かi2c12か
i2c_baud_rate : 通信速度  Hz  通常は400kHz以下を使う
i2c_gpios : i2cのSDAとSCLのピン番号，{ }の中に入れて，{SDA,SCL}の順番で並べて書く
i2c_slave_addr : 自身のスレーブアドレス
memory : グローバル変数!!  入出力するデータを保存しておく配列へのポインタ
*/
void SetupI2cSlave(bool i2c_num, uint32_t i2c_baud_rate, std::initializer_list<uint8_t> i2c_gpios, uint8_t i2c_slave_addr, uint8_t *memory) {
    set_memory(i2c_num, memory);

    for (uint8_t i2c_gpio : i2c_gpios) {
        gpio_init(i2c_gpio);
        gpio_set_function(i2c_gpio, GPIO_FUNC_I2C);
        gpio_pull_up(i2c_gpio);
    }

    if (i2c_num) {
        i2c_init(i2c1, i2c_baud_rate);
        i2c_slave_init(i2c1, i2c_slave_addr, &I2cSlaveHandler);
    } else {
        i2c_init(i2c0, i2c_baud_rate);
        i2c_slave_init(i2c0, i2c_slave_addr, &I2cSlaveHandler);
    }

    sleep_ms(10);  // 要検証
}

/*
I2Cをスレーブとしてセットアップする (送受信にレジスタアドレスを使用しない)
事前に渡された配列のデータを出力，それとは別の事前に渡された配列に書き込みを行います．
i2c_num : i2c0かi2c12か
i2c_baud_rate : 通信速度  Hz  通常は400kHz以下を使う
i2c_gpios : i2cのSDAとSCLのピン番号，{ }の中に入れて，{SDA,SCL}の順番で並べて書く
i2c_slave_addr : 自身のスレーブアドレス
input_data : グローバル変数!!  入力するデータを保存しておく配列へのポインタ
output_data : グローバル変数!!  出力するデータを保存しておく配列へのポインタ
*/
void SetupI2cSlave_Direct(bool i2c_num, uint32_t i2c_baud_rate, std::initializer_list<uint8_t> i2c_gpios, uint8_t i2c_slave_addr, uint8_t *input_data, uint8_t *output_data) {
    set_input_data(i2c_num, input_data);
    set_output_data(i2c_num, output_data);

    for (uint8_t i2c_gpio : i2c_gpios) {
        gpio_init(i2c_gpio);
        gpio_set_function(i2c_gpio, GPIO_FUNC_I2C);
        gpio_pull_up(i2c_gpio);
    }

    if (i2c_num) {
        i2c_init(i2c1, i2c_baud_rate);
        i2c_slave_init(i2c1, i2c_slave_addr, &I2cSlaveHandler_Direct);
    } else {
        i2c_init(i2c0, i2c_baud_rate);
        i2c_slave_init(i2c0, i2c_slave_addr, &I2cSlaveHandler_Direct);
    }

    sleep_ms(10);  // 要検証
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://omoroya.com/arduino-extra-edition-06/
https://marycore.jp/prog/cpp/variadic-function/
pico_sdkのサンプルコード"slave_mem_i2c.c"
http://vivi.dyndns.org/tech/cpp/static.html
*/