// 未検証
#include "i2c_s.hpp"

/*
I2C通信を簡単に行うためのクラスです
*/

// I2Cのセットアップ  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
I2c::I2c(bool i2c_id, uint8_t scl_gpio, uint8_t sda_gpio, uint32_t i2c_freq)
{
    i2c_id_ = i2c_id;
    if(i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0)  // すでにI2Cをセットしたかを判定
    {
        if (!i2c_id_ ? kAlreadyUseI2c1 : kAlreadyUseI2c0)
        {
            i2c_id_ = !i2c_id_;  // まだセットされていないI2Cがあるなら，セットされていない方に切り換える
            Log("The specified I2C ID was already in use, so it was switched to an unused one.");  // 指定されたI2CのIDがすでに使われていたため，使われていない方に切り替えました
        } else {
            throw Error(__FILE__, __LINE__, "I2C with the same ID cannot be initialized twice");  //同じIDのI2Cを二回初期化することはできません
        }
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
    int read_bytes = 0;
    if (i2c_id_)
    {
        read_bytes = i2c_read_blocking(i2c1, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        read_bytes = i2c_read_blocking(i2c0, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    // 正常に読み取れたかを確認
    if (read_bytes == PICO_ERROR_GENERIC)
    {
        throw Error(__FILE__, __LINE__, "Communication by I2C is not possible. Either the address is not recognized or the device does not exist.");  // I2Cによる通信ができません．アドレスが認識されていないか、デバイスが存在しません．
    } else if (read_bytes != input_data_bytes) {
        // データを一部しか読み取れなかった場合は，残りを再度読み込み
        if (0 <= read_bytes && read_bytes < input_data_bytes)
        {
            sleep_ms(10);
            int read_bytes2 = 0;
            if (i2c_id_)
            {
                read_bytes2 = i2c_read_blocking(i2c1, slave_addr, &(input_data[read_bytes]), input_data_bytes - read_bytes, false);
            } else {
                read_bytes2 = i2c_read_blocking(i2c0, slave_addr, &(input_data[read_bytes]), input_data_bytes - read_bytes, false);
            }
            if (read_bytes + read_bytes2 < input_data_bytes)
                throw Error(__FILE__, __LINE__, "Only some data could be read during I2C communication");  // I2Cによる通信にて，一部のデータしか読み取れませんでした
        } else {
            throw Error(__FILE__, __LINE__, "Communication via I2C failed");  // I2Cによる通信に失敗しました
        }

    }
    
}

//  I2Cで受信 (メモリから)
void I2c::ReadMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *input_data, std::size_t input_data_bytes) {
        int read_bytes = 0, written_bytes = 0;
        if (i2c_id_)
        {
            written_bytes = i2c_write_blocking(i2c1, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
            read_bytes = i2c_read_blocking(i2c1, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        } else {
            written_bytes = i2c_write_blocking(i2c0, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
            read_bytes = i2c_read_blocking(i2c0, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        }

    // 正常に読み取れたかを確認
    if (read_bytes == PICO_ERROR_GENERIC || written_bytes == PICO_ERROR_GENERIC)
    {
        throw Error(__FILE__, __LINE__, "Communication by I2C is not possible. Either the address is not recognized or the device does not exist.");  // I2Cによる通信ができません．アドレスが認識されていないか、デバイスが存在しません．
    } else if (read_bytes != input_data_bytes) {
        // データを一部しか読み取れなかった場合は，残りを再度読み込み
        if (0 <= read_bytes && read_bytes < input_data_bytes)
        {
            sleep_ms(10);
            int read_bytes2 = 0;
            if (written_bytes == 0)
                written_bytes = i2c_write_blocking((i2c_id_ ? i2c1 : i2c0), slave_addr, &memory_addr, 1, true);  // まだメモリアドレスを送信していなかった場合は，再度送信を試みる
            if (i2c_id_)
            {
                read_bytes2 = i2c_read_blocking(i2c1, slave_addr, &(input_data[read_bytes]), input_data_bytes - read_bytes, false);
            } else {
                read_bytes2 = i2c_read_blocking(i2c0, slave_addr, &(input_data[read_bytes]), input_data_bytes - read_bytes, false);
            }
            if (written_bytes != 1)
                throw Error(__FILE__, __LINE__, "Failed to write memory address in I2C communication.");  // I2Cによる通信にて，メモリアドレスの書き込みに失敗しました
            if (read_bytes + read_bytes2 < input_data_bytes)
                throw Error(__FILE__, __LINE__, "Only some data could be read during I2C communication");  // I2Cによる通信にて，一部のデータしか読み取れませんでした
        } else {
            throw Error(__FILE__, __LINE__, "Communication via I2C failed");  // I2Cによる通信に失敗しました
        }
    }
}

//  I2Cで送信 (マイコンなどへ)
void I2c::Write(uint8_t slave_addr, uint8_t *output_data, std::size_t output_data_bytes) {
    int written_bytes = 0;
    if (i2c_id_)
    {
        written_bytes = i2c_write_blocking(i2c1, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        written_bytes = i2c_write_blocking(i2c0, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    // 正常に書き込めたかを確認
    if (written_bytes == PICO_ERROR_GENERIC)
    {
        throw Error(__FILE__, __LINE__, "Communication by I2C is not possible. Either the address is not recognized or the device does not exist.");  // I2Cによる通信ができません．アドレスが認識されていないか、デバイスが存在しません．
    } else if (written_bytes != output_data_bytes) {
        // データを一部しか書き込めなかった場合は，残りを再度書き込み
        if (0 <= written_bytes && written_bytes < output_data_bytes)
        {
            sleep_ms(10);
            int written_bytes2 = 0;
            if (i2c_id_)
            {
                written_bytes2 = i2c_write_blocking(i2c1, slave_addr, &(output_data[written_bytes]), output_data_bytes - written_bytes, false);
            } else {
                written_bytes2 = i2c_write_blocking(i2c0, slave_addr, &(output_data[written_bytes]), output_data_bytes - written_bytes, false);
            }
            if (written_bytes + written_bytes2 < output_data_bytes)
                throw Error(__FILE__, __LINE__, "Only some data could be write during I2C communication");  // I2Cによる通信にて，一部のデータしか書き込めませんでした
        } else {
            throw Error(__FILE__, __LINE__, "Communication via I2C failed");  // I2Cによる通信に失敗しました
        }

    }
}

// I2Cで送信 (メモリへ)
void I2c::WriteMem(uint8_t slave_addr, uint8_t memory_addr, uint8_t *output_data, std::size_t output_data_bytes) {
    int written_bytes = 0, written_bytes_a = 0;
    if (i2c_id_)
    {
        written_bytes_a = i2c_write_blocking(i2c1, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        written_bytes = i2c_write_blocking(i2c1, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    } else {
        written_bytes_a = i2c_write_blocking(i2c0, slave_addr, &memory_addr, 1, true);  // メモリアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        written_bytes = i2c_write_blocking(i2c0, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
    }

    // 正常に書き込めたかを確認
    if (written_bytes == PICO_ERROR_GENERIC || written_bytes_a == PICO_ERROR_GENERIC)
    {
        throw Error(__FILE__, __LINE__, "Communication by I2C is not possible. Either the address is not recognized or the device does not exist.");  // I2Cによる通信ができません．アドレスが認識されていないか、デバイスが存在しません．
    } else if (written_bytes != output_data_bytes) {
        // データを一部しか書き込めなかった場合は，残りを再度書き込み
        if (0 <= written_bytes && written_bytes < output_data_bytes)
        {
            sleep_ms(10);
            int written_bytes2 = 0;
            if (written_bytes_a == 0)
                written_bytes_a = i2c_write_blocking((i2c_id_ ? i2c1 : i2c0), slave_addr, &memory_addr, 1, true);  // まだメモリアドレスを送信していなかった場合は，再度送信を試みる
            if (i2c_id_)
            {
                written_bytes2 = i2c_read_blocking(i2c1, slave_addr, &(output_data[written_bytes]), output_data_bytes - written_bytes, false);
            } else {
                written_bytes2 = i2c_read_blocking(i2c0, slave_addr, &(output_data[written_bytes]), output_data_bytes - written_bytes, false);
            }
            if (written_bytes_a != 1)
                throw Error(__FILE__, __LINE__, "Failed to write memory address in I2C communication.");  // I2Cによる通信にて，メモリアドレスの書き込みに失敗しました
            if (written_bytes + written_bytes2 < output_data_bytes)
                throw Error(__FILE__, __LINE__, "Only some data could be write during I2C communication");  // I2Cによる通信にて，一部のデータしか書き込めませんでした
        } else {
            throw Error(__FILE__, __LINE__, "Communication via I2C failed");  // I2Cによる通信に失敗しました
        }
    }
}


/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://omoroya.com/arduino-extra-edition-06/
https://marycore.jp/prog/cpp/variadic-function/
pico_sdkのサンプルコード"slave_mem_i2c.c"
http://vivi.dyndns.org/tech/cpp/static.html
*/