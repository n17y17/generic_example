#include "i2c_a.hpp"

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
        Wire1.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire1.write(reg);  // レジスタアドレス(スレーブのメモリの何番地から読み込むか)を先に送信
        Wire1.endTransmission(false);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
        Wire1.requestFrom(addr, len, true);  // スレーブアドレスと要求するバイト数を指定し，データを受信する  第3引数は，停止信号を送りI2Cバスを開放するか
        while (Wire1.available() < len) ;  // 全バイトが読み取り可能になるまで待機
        while (len--) *(data++) = Wire1.read();  // 受信したデータを読み取って保存
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

/*
スレーブからの読み込み (レジスタアドレスを指定しない)
i2c_num : i2c0かi2c1か
addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
data : 受信したデータを入れるための変数
len : 何バイト(文字)読み込むか
*/
void ReadI2c_Direct(bool i2c_num, uint8_t addr, uint8_t *data, size_t len) {
    if (i2c_num) {
        Wire1.requestFrom(addr, len, true);  // スレーブアドレスと要求するバイト数を指定し，データを受信する  第3引数は，停止信号を送りI2Cバスを開放するか
        while (Wire1.available() < len) ;  // 全バイトが読み取り可能になるまで待機
        while (len--) *(data++) = Wire1.read();  // 受信したデータを読み取って保存
    } else {
        Wire.requestFrom(addr, len, true);  // スレーブアドレスと要求するバイト数を指定し，データを受信する  第3引数は，停止信号を送りI2Cバスを開放するか
        while (Wire.available() < len) ;  // 全バイトが読み取り可能になるまで待機
        while (len--) *(data++) = Wire.read();  // 受信したデータを読み取って保存
    }

    delay(10);  // 要検証
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
        Wire1.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire1.write(reg);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        Wire1.write(data, len);  // バイト数を指定してデータを送信  (Wire.write("test");でも送れる)
        Wire1.endTransmission(true);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
    } else {
        Wire.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire.write(reg);  // レジスタアドレス(スレーブのメモリの何番地に書き込むか)を先に送信
        Wire.write(data, len);  // バイト数を指定してデータを送信  (Wire.write("test\n");でも送れる)
        Wire.endTransmission(true);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
    }

    delay(10);  // 要検証
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
        Wire1.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire1.write(data, len);  // バイト数を指定してデータを送信  (Wire.write("test");でも送れる)
        Wire1.endTransmission(true);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
    } else {
        Wire.beginTransmission(addr);  // 接続を開始  引数は送信先のスレーブアドレス
        Wire.write(data, len);  // バイト数を指定してデータを送信  (Wire.write("test\n");でも送れる)
        Wire.endTransmission(true);  //送信を完了して，接続を終了  引数は，停止信号を送りI2Cバスを開放するか
    }

    delay(10);  // 要検証
}

// ----ここから，スレーブ用の関数-----

// 変数の受け渡し用
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
(レジスタアドレスを受信しない)
*/
static void I2c0ReceiveHandler_Direct(int len_read) {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kInputData = set_input_data(0);
    kInputData = {0};

    while (Wire.available() < len_read) ;  // 全バイトが読み取り可能になるまで待機
    while (len_read--) *(kInputData++) = Wire.read();  // 受信したデータを読み取って保存
    Serial.print(">");
    Serial.println((char*)kInputData);
}
static void I2c1ReceiveHandler_Direct(int len_read) {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kInputData = set_input_data(1);
    kInputData = {0};

    while (Wire.available() < len_read) ;  // 全バイトが読み取り可能になるまで待機
    while (len_read--) *(kInputData++) = Wire.read();  // 受信したデータを読み取って保存
    Serial.print(">");
    Serial.println((char*)kInputData);
}

/*
マスターに送信するとき，この関数が割り込み処理で実行される
(レジスタアドレスを受信しない)
*/
static void I2c0RequestHandler_Direct() {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kOutputData = set_output_data(0);

    Wire.write((char*)kOutputData);  // データを送信
    Serial.print("<");
    Serial.println((char*)kOutputData);
}
static void I2c1RequestHandler_Direct() {
    // staticを付けたローカル変数は最初に呼び出された時だけ初期化され，その後も値は保持される
    static uint8_t *kOutputData = set_output_data(1);

    Wire.write((char*)kOutputData);  // データを送信
    Serial.print("<");
    Serial.println((char*)kOutputData);
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

    int i = 0;
    if (i2c_num) {
        for (uint8_t i2c_gpio : i2c_gpios) {
            if ((++i) % 2) Wire1.setSDA(i2c_gpio);
            else Wire1.setSCL(i2c_gpio);
        }
        Wire1.begin(i2c_slave_addr);  // スレーブとして接続
        Wire1.setClock(i2c_baud_rate);
        Wire1.onReceive(I2c1ReceiveHandler_Direct);  // マスターから受信した際に呼び出される関数を設定
        Wire1.onRequest(I2c1RequestHandler_Direct);  // マスターからリクエストを受けた際に呼び出される関数を設定
    } else {
        for (uint8_t i2c_gpio : i2c_gpios) {
            if ((++i) % 2) Wire.setSDA(i2c_gpio);
            else Wire.setSCL(i2c_gpio);
        }
        Wire.begin(i2c_slave_addr);  // スレーブとして接続
        Wire.setClock(i2c_baud_rate);
        Wire.onReceive(I2c0ReceiveHandler_Direct);  // マスターから受信した際に呼び出される関数を設定
        Wire.onRequest(I2c0RequestHandler_Direct);  // マスターからリクエストを受けた際に呼び出される関数を設定
    }

    delay(10);  // 要検証
}

/*
このプログラムの作成にあたり以下を参考にしました
https://stemship.com/arduino-beginner-i2c/
https://omoroya.com/arduino-extra-edition-06/
https://marycore.jp/prog/cpp/variadic-function/
pico_sdkのサンプルコード"slave_mem_i2c.c"
http://vivi.dyndns.org/tech/cpp/static.html
*/