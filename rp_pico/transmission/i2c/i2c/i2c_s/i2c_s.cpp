// 未検証
#include "i2c_s.hpp"

/*
I2C通信を簡単に行うためのライブラリです
pico-SDKを使用しています
*/

/*
-----I2C通信とは-----
I2C通信は1台のマスターと複数台のスレーブの間の通信です．
通信はデータ通信線(SDA)と，クロック信号線(CLK)により行います．
！マスターの二つのピンだけで，複数のセンサに繋げます！
master   slave1   slave2  ...
 SDA <==> SDA <==> SDA <==> ...
 SCL ===> SCL ===> SCL ===> ...

マスターはスレーブに自由にデータを送信できますが，
スレーブはマスターからの指示を受信したときのみ送信できます．(マスターが送るクロック信号を基準にして通信する)

各スレーブには，スレーブアドレス(マスターが指示を送るための，デバイスごとのアドレス)が割り当てられています．
デバイスの各メモリにはメモリアドレス(=レジスタアドレス)(メモリ内の番地を表すアドレス)が割り当てられています．

マスターは各通信の最初に，これから通信したいスレーブのメモリアドレスを送信します．(通信相手のメモリの何番地に書き込む/読み込むかを指定する)
(このとき，スレーブアドレスも一緒に送り誰宛の通信かも分かるようにしているが，あまり意識しなくてもOK)
その後に，マスターが送信しようとした場合，スレーブは初めに送られてきたメモリアドレスの場所にデータを書き込みます．
　　　　　マスターが受信しようとした場合，スレーブは初めに送られてきたメモリアドレスの場所のデータを送信します．
*/

bool I2c::kAlreadyUseI2c0 = false;
bool I2c::kAlreadyUseI2c1 = false;

// -----ここから，マスター用の関数-----

// I2Cのセットアップ  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
I2c::I2c(bool i2c_id, uint8_t scl_gpio, uint8_t sda_gpio, uint32_t i2c_freq) {
    i2c_id_ = i2c_id;
    if(i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0) {
        // エラー  同じIDのI2Cを二回初期化することはできません
    } else {
        (i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0) = true;
    }

    i2c_init((i2c_id_ ? i2c1 : i2c0), i2c_freq);  // I2Cの初期化

    gpio_set_function(scl_gpio, GPIO_FUNC_I2C);  // GPIOピンの有効化
    gpio_pull_up(scl_gpio);
    gpio_set_function(sda_gpio, GPIO_FUNC_I2C);  // GPIOピンの有効化
    gpio_pull_up(sda_gpio);
    
    sleep_ms(10);  // 要検証
}

//I2Cで受信 (マイコンなどから)
void I2c::Read(uint8_t slave_addr, uint8_t *input_data, std::size_t input_data_bytes) {
    if (i2c_id_) {
        i2c_read_blocking(i2c1, slave_addr, (uint8_t*)input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_read_blocking(i2c0, slave_addr, (uint8_t*)input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

//  I2Cで受信 (メモリから)
void I2c::ReadMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *input_data, std::size_t input_data_bytes) {
    if (i2c_id_) {
        i2c_write_blocking(i2c1, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        i2c_read_blocking(i2c1, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        i2c_read_blocking(i2c0, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

//  I2Cで送信 (マイコンなどへ)
void I2c::Write(uint8_t slave_addr, uint8_t *output_data, std::size_t output_data_bytes) {
    if (i2c_id_) {
        i2c_write_blocking(i2c1, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}

// I2Cで送信 (メモリへ)
void I2c::WriteMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *output_data, std::size_t output_data_bytes) {
    if (i2c_id_) {
        i2c_write_blocking(i2c1, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c1, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        i2c_write_blocking(i2c0, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        i2c_write_blocking(i2c0, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    sleep_ms(10);  // 要検証
}



// ----ここから，スレーブ用の関数-----

static std::deque<uint8_t> kInputData0(0);
static std::deque<uint8_t> kOutputData0(0);
static std::deque<uint8_t> kInputData1(0);
static std::deque<uint8_t> kOutputData1(0);

/*
マスターからの受信があったとき，この関数が割り込み処理で実行される
(メモリアドレスを受信しない)
*/
static void I2cSlaveHandler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    std::deque<uint8_t> &input_data = (i2c == i2c1 ? kInputData1 : kInputData0);
    std::deque<uint8_t> &output_data = (i2c == i2c1 ? kInputData1 : kInputData0);
    switch (event) {
        case I2C_SLAVE_RECEIVE: {  // マスターがデータを書き込んだとき
            input_data.push_back(i2c_read_byte_raw(i2c));  // 読み込んだデータを保存
            printf(" >%c", input_data.back());
            break;
        }
        case I2C_SLAVE_REQUEST: {  // マスターがデータを要求しているとき
            if (output_data.size()) {
                i2c_write_byte_raw(i2c, output_data.front());  // データを送信
                printf(" <%c", output_data.front());
                output_data.pop_front();
            } else {
                // エラー  マスターからデータ送信のリクエストが来ていますが，送信できるデータがありません
            }
        break;
        }
        case I2C_SLAVE_FINISH: {  // マスターが停止／再起動の信号を送信したとき
            printf("\n");
            break;
        }
        default: {
            break;
        }
    }
}

// すでに受信した，読み取り可能なデータのバイト数を返す
std::size_t I2c::DataReceivable() {
    if (i2c_id_) {
        return kInputData1.size();
    } else {
        return kInputData0.size();
    }
    
}

//  マスターから受信し，一時保存してあったたデータを読み取る
std::size_t I2c::GetInputData(uint8_t *input_data, std::size_t input_data_bytes) {
    int i = 0;
    if (i2c_id_) {
        while (kInputData1.size() && i < input_data_bytes) {
            input_data[i++] = kInputData1.front();
            kInputData1.pop_front();
        }
    } else {
        while (kInputData0.size() && i < input_data_bytes) {
            input_data[i++] = kInputData0.front();
            kInputData0.pop_front();
        }
    }
    return i;
}

//  マスターに送信するためのデータをセットする
void I2c::SetOutputData(uint8_t *output_data, std::size_t output_data_bytes) {
    int i = 0;
    if (i2c_id_) {
        while (i < output_data_bytes) {
            kOutputData1.push_back(output_data[i++]);
        }
    } else {
        while (i <  output_data_bytes) {
            kOutputData0.push_back(output_data[i++]);
        }
    }
}

//  I2Cをスレーブとしてセットアップ (送受信にメモリアドレスを使用しない)
I2c::I2c(bool i2c_id, uint8_t scl_gpio, uint8_t sda_gpio, uint32_t i2c_freq, uint8_t slave_addr) {
    i2c_id_ = i2c_id;
    if(i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0) {
        // エラー  同じIDのI2Cを二回初期化することはできません
    } else {
        (i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0) = true;
    }

    gpio_init(scl_gpio);
    gpio_set_function(scl_gpio, GPIO_FUNC_I2C);  // 使用するピンを有効化
    gpio_pull_up(scl_gpio);
    gpio_init(sda_gpio);
    gpio_set_function(sda_gpio, GPIO_FUNC_I2C);  // 使用するピンを有効化
    gpio_pull_up(sda_gpio);

    if (i2c_id_) {
        i2c_init(i2c1, i2c_freq);
        i2c_slave_init(i2c1, slave_addr, &I2cSlaveHandler);  // I2Cをスレーブに設定  通信時の割り込み処理を設定
    } else {
        i2c_init(i2c0, i2c_freq);
        i2c_slave_init(i2c0, slave_addr, &I2cSlaveHandler);  // I2Cをスレーブに設定  通信時の割り込み処理を設定
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