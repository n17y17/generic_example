#ifndef SC_PROJECT_RP_PICO_PICO_SDK_SC_SC_HPP_
#define SC_PROJECT_RP_PICO_PICO_SDK_SC_SC_HPP_

#define _USE_MATH_DEFINES  // 円周率などの定数を使用する  math.hを読み込む前に定義する必要がある (math.hはcmathやiostreamに含まれる)
#include <cfloat>
#include <cmath>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <string>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/pwm.h"

// Can Sat でよく使うセンサやモータードライバを簡単に使用するためのライブラリです．
namespace sc
{
    /**************************************************/
    /********************ログ・エラー*******************/
    /**************************************************/

    // ログを記録する関数です．
    // ライブラリの使用前に外部で定義してください
    // 末尾に改行を追加する必要はありません
    void save_log(const std::string& log);

    // エラーを記録し，標準エラー出力に出力します
    class Error : public std::exception
    {
        std::string _message;

    public:
        // エラーを記録し，標準エラー出力に出力します
        // FILE は＿FILE＿，LINEは＿LINE＿としてください． (自動でファイル名と行番号に置き換わります)
        // message : 出力したいエラーメッセージ (自動で改行)
        Error(const std::string& FILE, int LINE, const std::string& message) noexcept;

        // エラーを記録し，標準エラー出力に出力します
        // FILE は＿FILE＿，LINEは＿LINE＿としてください． (自動でファイル名と行番号に置き換わります)
        // message : 出力したいエラーメッセージ (自動で改行)
        // what : キャッチした例外のエラーメッセージ (e.what())
        Error(const std::string& FILE, int LINE, const std::string& message, const std::string& what) noexcept:
            _message(message)
        {
            Error(FILE, LINE, _message + "   " + what);
        }

        const char* what() const noexcept {return _message.c_str();}
    };

    // ログを記録し，標準出力に出力します
    void log(const std::string& message) noexcept;

    // ゼロ除算防止
    template<typename T> inline T not0(T value) {return (value ? value : 1);}
    template<> inline float not0(float value) {return (value ? value : 1e-10);}
    template<> inline double not0(double value) {return (value ? value : 1e-10);}
    template<> inline long double not0(long double value) {return (value ? value : 1e-10);}

    // コピーコンストラクタを禁止するための親クラス
    class Noncopyable
    {
    protected:
        Noncopyable() = default;
        ~Noncopyable() = default;
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable& operator=(const Noncopyable&) = delete;
    };
    // Noncopyableクラスは以下の資料を参考にして作成しました
    // https://cpp.aquariuscode.com/uncopyable-mixin

    
    /**************************************************/
    /*****************測定値および変換******************/
    /**************************************************/

    constexpr double ErrorValue = -1024.0;  // 測定できなかったとき，測定しなかったときのデフォルトの測定値

    // 測定値を保存するための親クラス
    // 実質的にdouble型として使用可能
    // ただし，Temperature型が引数の関数にPressure型の値を代入しようとするなどするとコンパイルエラーになる
    class Measurement
    {
        double _value;
    public:
        Measurement(double value) : _value(value) {}
        Measurement() : Measurement(ErrorValue) {}
        operator double&() {return _value;}
        operator const double&() const {return _value;}
        Measurement& operator= (double value) {_value = value; return *this;}
    };

    struct X : Measurement {using Measurement::Measurement;};  // XYZの3つの値を持つ量のうちのXの値
    struct Y : Measurement {using Measurement::Measurement;};  // XYZの3つの値を持つ量のうちのYの値
    struct Z : Measurement {using Measurement::Measurement;};  // XYZの3つの値を持つ量のうちのZの値
    struct Magnitude : Measurement {using Measurement::Measurement;};  // ベクトル量の大きさ
    struct A_Direction : Measurement {using Measurement::Measurement;};  // 絶対的(Absolute)な方角 (°)  重力の方向と垂直な面における角度  0°:北, 90°:東, 180°:南, 270°:西
    struct R_Direction : Measurement {using Measurement::Measurement;};  // 相対的(Relative)な方角 (°)  機体のある面における角度  0°:前, 90°:右, 180°:後, 270°:左
    struct A_Elevation : Measurement {using Measurement::Measurement;};  // 絶対的(Absolute)な仰角 (°)  水平面に対する角度(-180°<, <=180°)  A_Directionの方向を向いた状態から天頂方向に何度傾いた角度であるか
    struct R_Elevation : Measurement {using Measurement::Measurement;};  // 相対的(Relative)な仰角 (°)  機体のある平面に対する角度(-180°<, <=180°)  R_Directionの方向を向いた状態から，機体に対しての真上方向に何度傾いた角度であるか
    struct Altitude : Measurement {using Measurement::Measurement;};  // 標高 (m)  海水面(ジオイド)からの高さ
    struct Temperature : Measurement {using Measurement::Measurement;};  // 気温 (℃)
    struct Pressure : Measurement {using Measurement::Measurement;};  // 気圧 (hPa)
    struct Humidity : Measurement {using Measurement::Measurement;};  // 湿度 (%)
    struct Latitude : Measurement {using Measurement::Measurement;};  // 緯度 (dd.dd...°)  正:北緯, 負:南緯
    struct Longitude : Measurement {using Measurement::Measurement;};  // 経度 (dd.dd...°)  正:東経, 負:西経
    struct Distance : Measurement {using Measurement::Measurement;};  // 距離 (m)
    struct Area : Measurement {using Measurement::Measurement;};  // 面積 (カメラを想定) (px)

    struct A_Position  // 絶対(Absolute)直交座標上の位置 (m)  電源を入れた位置を原点とし，+y:北, +x:東, +z:天頂 とする
    {
        X x = ErrorValue;
        Y y = ErrorValue;
        Z z = ErrorValue;
        A_Position() {}
        A_Position(X x_, Y y_, Z z_) : x(x_), y(y_), z(z_) {}
        A_Position(X x_, Y y_) : A_Position(x_, y_, 0.0) {}
        explicit A_Position(double value) : A_Position(value, value, value) {}
    };
    struct R_Position  // 相対(Relative)直交座標上の位置 (m)  今の機体の位置を原点とし，前:+y, 右:+x, 上:+z とする
    {
        X x = ErrorValue;
        Y y = ErrorValue;
        Z z = ErrorValue;
        R_Position() {}
        R_Position(X x_, Y y_, Z z_) : x(x_), y(y_), z(z_) {}
        explicit R_Position(double value) : R_Position(value, value, value) {}
    };

    struct VectorXYZ  // XYZの成分を持つベクトル量  今の機体の位置を原点とし，前:+y, 右:+x, 上:+z とする
    {
        X x = ErrorValue;
        Y y = ErrorValue;
        Z z = ErrorValue;
        VectorXYZ() {}
        VectorXYZ(X x_, Y y_, Z z_) : x(x_), y(y_), z(z_) {}
        explicit VectorXYZ(double value) : VectorXYZ(value, value, value) {}
    };
    struct Acceleration : VectorXYZ {using VectorXYZ::VectorXYZ;};  // 加速度 (x, y, z) (m/s/s)
    struct GravityAcceleration : Acceleration {using Acceleration::Acceleration;};  // 重力加速度 (x, y, z) (m/s/s)
    struct LineAcceleration : Acceleration {using Acceleration::Acceleration;};  // 重力を除いた加速度 (x, y, z) (m/s/s)
    struct Gyro : VectorXYZ {using VectorXYZ::VectorXYZ;};  // 角速度 (x, y, z) (rad/s)
    struct Magnetism : VectorXYZ {using VectorXYZ::VectorXYZ;};  // 磁気 (x, y, z) (mT)

    struct A_Angle  // 空間における向き (°)  方位に対する絶対的(Absolute)な向き
    {
        A_Direction direction = ErrorValue;  // 絶対的(Absolute)な方角 (°)  重力の方向と垂直な面における角度  0°:北, 90°:東, 180°:南, 270°:西
        A_Elevation elevation = ErrorValue;  // 絶対的(Absolute)な仰角 (°)  水平面に対する角度(-180°<, <=180°)  A_Directionの方向を向いた状態から天頂方向に何度傾いた角度であるか
    };
    struct R_Angle  // 空間における向き (°)  機体に対する相対的(Relative)な向き
    {
        R_Direction direction = ErrorValue;  // 相対的(Relative)な方角 (°)  機体のある面における角度  0°:前, 90°:右, 180°:後, 270°:左
        R_Elevation elevation = ErrorValue;  // 相対的(Relative)な仰角 (°)  機体のある平面に対する角度(-180°<, <=180°)  R_Directionの方向を向いた状態から，機体に対しての真上方向に何度傾いた角度であるか
    };
    

    #ifndef M_PI  // もしM_PIが定義されていなかったら
    constexpr double M_PI = 3.14159265358979323846;  // 円周率 PI
    #endif

    // ラジアンを度に変換
    inline double to_deg(double rad) {return rad / M_PI * 180.0;}

    // 度をラジアンに変換
    inline double to_rad(double deg) {return deg / 180.0 * M_PI;}

    // エラー値が含まれているかを判断
    // values : エラー値であるかを確かめる値  {1.0, 2.0, 3.0}のように複数の値を入力
    bool is_error(std::initializer_list<Measurement> values);

    // エラー値が含まれているかを判断
    // value : エラー値であるを確かめる値
    inline bool is_error(Measurement value) {return is_error({value});}

    // XYZの3つの値から大きさを計算
    inline Magnitude to_magnitude(VectorXYZ xyz) {return pow(xyz.x*xyz.x + xyz.y*xyz.y + xyz.z*xyz.z, 0.5);}

    // 標高の基準点をセット
    // pressure0 : 標高の基準点の気圧
    // [temperature0] : 標高の基準点の気温 (省略時:25.0)
    // [altitude0] : 標高の基準点の標高 (省略時:0.0)
    void set_altitude0(const Pressure& pressure0, const Temperature& temperature0, const Altitude& altitude0);

    // 気温と気圧から標高を計算
    // pressure : 測定した気圧
    // temperature : 測定した気温
    // [pressure0] : 基準点の気圧  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude (const Pressure& pressure, const Temperature& temperature, const Pressure& pressure0, const Altitude& altitude0) noexcept;

    // 気圧から標高を計算
    // pressure : 測定した気圧
    // [pressure0] : 基準点の気圧  省略可
    // [temperature0] : 基準点の気温  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude (const Pressure& pressure, const Pressure& pressure0, const Temperature& temperature0, const Altitude& altitude0) noexcept;

    // 気温から標高を計算
    // ！非推奨！精度が悪いです
    // temperature : 測定した気温
    // [temperature0] : 基準点の気温  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude (const Temperature& temperature, const Temperature& temperature0, const Altitude& altitude0) noexcept;

    // 重力加速度から標高を計算
    // ！非推奨！精度が悪いです
    // gravity : 測定した重力加速度
    // [gravity0] : 基準点の重力加速度の大きさ (省略時:赤道における正規重力)
    // [altitude0] : 基準点の標高 (省略時: 0.0)
    Altitude to_altitude(const GravityAcceleration& gravity, const Magnitude& gravity0, const Altitude& altitude) noexcept;

    // 重力加速度から標高を計算
    // ！非推奨！精度が悪いです
    // gravity : 測定した重力加速度
    // latitude : 測定した地点の緯度
    Altitude to_altitude(const GravityAcceleration& gravity, const Latitude& latitude) noexcept;

    // XYZ直交座標の原点をセット
    // latitude0 : 原点の緯度
    // longitude0 : 原点の経度
    // [altitude0] : 原点の標高 (省略時:0.0)
    void set_position0(const Latitude& latitude0, const Longitude& longitude0, const Altitude& altitude0);

    // 緯度 経度 標高をXYZ直交座標へ変換
    // latitude : 緯度
    // longitude : 経度
    // [altitude] : 標高 (省略時:0.0)
    // [latitude0] : 原点の緯度
    // [longitude0] : 原点の経度
    // [altitude0] : 原点の標高
    A_Position to_position(const Latitude& latitude, const Longitude& longitude, const Altitude& altitude, const Latitude& latitude0, const Longitude& longitude0, const Altitude& altitude0) noexcept;

    // 緯度 経度 標高をXYZ直交座標へ変換 (高速，簡易版)
    // latitude : 緯度
    // longitude : 経度
    // [altitude] : 標高 (省略時:0.0)
    // [latitude0] : 原点の緯度
    // [longitude0] : 原点の経度
    // [altitude0] : 原点の標高
    A_Position to_position_simple(const Latitude& latitude, const Longitude& longitude, const Altitude& altitude, const Latitude& latitude0, const Longitude& longitude0, const Altitude& altitude0) noexcept;

    // 2点の座標から直線距離を計算
    // position : XYZ直交座標での位置  1つ目
    // position0 : XYZ直交座標での位置  2つ目
    Distance to_distance(const A_Position& position, const A_Position& position0) noexcept;

    // 2点の緯度と経度から，地球楕円体表面上の精密な距離を計算
    // latitude : 1つ目の位置の 緯度
    // longitude : 1つ目の位置の 経度
    // [latitude0] : 2つ目の位置の 緯度
    // [longitude0] : 2つ目の位置の 経度
    Distance to_distance(const Latitude& latitude, const Longitude& longitude, const Latitude& latitude0, const Longitude& longitude0) noexcept;

    // 2点の緯度と経度から，大円距離を計算 (地球を真球と考えた時の表面上の距離)
    // latitude : 1つ目の位置の 緯度
    // longitude : 1つ目の位置の 経度
    // [latitude0] : 2つ目の位置の 緯度
    // [longitude0] : 2つ目の位置の 経度
    Distance to_distance_simple(const Latitude& latitude, const Longitude& longitude, const Latitude& latitude0, const Longitude& longitude0) noexcept;

    // 2点の座標から方位を計算
    A_Angle to_angle(const A_Position& position, const A_Position& position0) noexcept;

    // 2つの方位から相対的な向きを計算
    R_Angle to_r_angle(const A_Angle& angle, const A_Angle& angle0) noexcept;

    // 相対的な向きと基準の方位から方位を計算

    /**************************************************/
    /**********************センサ**********************/
    /**************************************************/

    // センサに関するクラスの親クラス
    class Sensor : private Noncopyable
    {
    public:
        // 以下の関数は例外を投げません
        // エラーが発生した場合はセンサの測定値がErrorValueになります

        virtual void measure() noexcept = 0;
        virtual bool check_connection() noexcept = 0;

        virtual Altitude altitude() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return altitude"); return ErrorValue;}  // このセンサは標高を返すことができません
        virtual Temperature temperature() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return temperature"); return ErrorValue;}  // このセンサは気温を返すことができません
        virtual Pressure pressure() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return pressure"); return ErrorValue;}  // このセンサは気圧を返すことができません
        virtual Humidity humidity() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return humidity"); return ErrorValue;}  // このセンサは湿度を返すことができません
        virtual Latitude latitude() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return latitude"); return ErrorValue;}  // このセンサは緯度を返すことができません
        virtual Longitude longitude() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return longitude"); return ErrorValue;}  // このセンサは経度を返すことができません
        virtual Distance distance() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return distance"); return ErrorValue;}  // このセンサは距離を返すことができません
        virtual Area area() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return area"); return ErrorValue;}  // このセンサは面積を返すことができません
        virtual Acceleration acceleration() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return acceleration"); return Acceleration(ErrorValue);}  // このセンサは加速度を返すことができません
        virtual Gyro gyro() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return gyro"); return Gyro(ErrorValue);}  // このセンサは角速度を返すことができません
        virtual Magnetism magnetism() const noexcept {Error(__FILE__, __LINE__, "This sensor cannot return magnetism"); return Magnetism(ErrorValue);}  // このセンサは磁気を返すことができません
    };


    /**************************************************/
    /***********************通信***********************/
    /**************************************************/

    // ピンの設定
    class Pin
    {
    public:
        // ピンのモード
        enum Mode
        {
            MODE_NO_CHANGE = -1,  // 変更しない
            IN,  // 入力用
            OUT  // 出力用
        };

        // ピンのプルアップ設定
        enum Pull
        {
            PULL_NO_CHANGE = -1,  // 変更しない
            PULL_NONE,  // 不使用
            PULL_UP,  // プルアップ
            PULL_DOWN  // プルダウン
        };

        // ピンの設定
        // gpio : ピンのGPIO番号
        // [mode_] : 出力用か，入力用か (省略時:変更しない)
        // [pull_] : ピンのプルアップ・プルダウン設定 (省略時:変更しない)
        Pin(uint8_t gpio, Mode mode_ = MODE_NO_CHANGE, Pull pull_ = PULL_NO_CHANGE);

        // ピンを初期化し，再設定
        // [mode_] : 出力用か，入力用か
        // [pull_] : ピンのプルアップ・プルダウン設定
        void init(Mode mode_ = MODE_NO_CHANGE, Pull pull_ = PULL_NO_CHANGE);

        // ピンのGPIO番号を取得
        uint8_t gpio() const;

        // ピンの入出力モードを取得
        Mode mode() const;

        // ピンの入出力モードを設定
        // mode_ : 出力用か，入力用か
        void mode(Mode mode_) const;

        // ピンのプルアップ・プルダウン状態を取得
        Pull pull() const;

        // ピンのプルアップ・プルダウン状態を設定
        // pull_ : ピンのプルアップ・プルダウン設定
        void pull(Pull pull_) const;

        // 出力用のピンの出力レベルを1にする
        // 出力モードになっていない場合の動作は未定義です
        void high() const;

        // 出力用のピンの出力レベルを0にする
        // 出力モードになっていない場合の動作は未定義です
        void low() const;

        // 入力用ピンの入力レベルを取得
        // 入力モードになっていない場合の動作は未定義です
        bool level() const;

        // 出力用ピンの出力レベルを設定
        // level_ : 出力レベル  0か1
        // 出力モードになっていない場合の動作は未定義です
        void level(bool level_) const;
    private:
        uint8_t _gpio;
    };

    // 通信に関するクラスの親クラス
    class Communication : Noncopyable
    {
    protected:
        static const uint8_t DeviceNotSelected = 255;  // デバイス指定用の値を指定しなかった場合のデフォルト値

    public:
        // 受信
        virtual void read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t select_device = DeviceNotSelected) const = 0;

        // 送信
        virtual void write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t select_device = DeviceNotSelected) const = 0;

        // メモリから読み込み
        // memory_addr : 相手のデバイスの何番地のメモリーからデータを読み込むか
        // input_data_bytes : 何バイト(文字)データを読み込むか (省略した場合は，input_dataの長さだけ読み込む)
        // input_data : 受信したデータを保存するための配列
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        // SPIの場合はメモリーアドレスの8ビット目を自動で0に置き換えます
        virtual void read_mem(uint8_t memory_addr, std::size_t input_data_bytes, uint8_t* input_data, uint8_t select_device = DeviceNotSelected) const
        {
            this->write(1, &memory_addr, select_device);
            this->read(input_data_bytes, input_data, select_device);
        }
        template<typename T, std::size_t Size> void read_mem(uint8_t memory_addr, T (&input_data)[Size], uint8_t select_device = DeviceNotSelected) const {read_mem(memory_addr, Size, (uint8_t*)input_data, select_device);}

        // メモリに書き込み
        // memory_addr : 相手のデバイスの何番地のメモリーにデータを書き込むか
        // output_data_bytes : 何バイト(文字)データを書き込むか (省略した場合は，output_dataの長さだけ書き込む)
        // output_data : 送信するデータの配列
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        // SPIの場合はメモリーアドレスの8ビット目を自動で1に置き換えます
        virtual void write_mem(uint8_t memory_addr, std::size_t output_data_bytes, uint8_t* output_data, uint8_t select_device = DeviceNotSelected) const
        {
            this->write(1, &memory_addr, select_device);
            this->write(output_data_bytes, output_data, select_device);
        }
        template<typename T, std::size_t Size> void write_mem(uint8_t memory_addr, T (&output_data)[Size], uint8_t select_device = DeviceNotSelected) const {write_mem(memory_addr, Size, (uint8_t*)output_data, select_device);}

        // 1バイトだけメモリに書き込み
        // memory_addr : 相手のデバイスの何番地のメモリーにデータを書き込むか
        // output_data : 送信する1バイトのデータ
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        void write_byte_mem(uint8_t memory_addr, uint8_t output_data, uint8_t select_device = DeviceNotSelected) const {this->write_mem(memory_addr, 1, &output_data, select_device);}

        // 改行までデータを読み込む
        // input_data_bytes : 最大で何バイト(文字)データを読み込むか (省略した場合は，最大でinput_dataの長さだけ読み込む)
        // input_data : 受信したデータを保存するための配列
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        void read_line(std::size_t input_data_bytes, uint8_t* input_data, uint8_t select_device = DeviceNotSelected) const;
        template<typename T, std::size_t Size> void read_line(T (&input_data)[Size], uint8_t select_device = DeviceNotSelected) const {read_line(Size, (uint8_t*)input_data, select_device);}
    };

    // I2C通信を行います
    class I2C : public Communication
    {
    public:
        // I2Cのセットアップ  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
        // i2c_id : I2C0かi2c1か
        // scl_pin : I2CのSCLのピン (ピン番号のみ指定したもの)
        // sda_pin : I2CのSDAのピン (ピン番号のみ指定したもの)
        // freq : I2Cの転送速度
        I2C(bool i2c_id, Pin scl_pin, Pin sda_pin, const uint32_t& freq);

        // I2Cで受信
        // input_data_bytes : 何バイト(文字)読み込むか (省略した場合はinput_dataの長さだけ読み取る)
        // input_data : 受信したデータを保存するための配列
        // slave_addr : 通信先のデバイスのCSピンのGPIO番号
        void read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t slave_addr) const;
        template<typename T, std::size_t Size> void read(T (&input_data)[Size], uint8_t slave_addr) const {read(Size, (uint8_t*)input_data, slave_addr);}

        // I2Cで送信
        // output_data_bytes : 何バイト(文字)書き込むか (省略した場合はoutput_dataの長さだけ書き込む)
        // output_data : 送信するデータの配列
        // slave_addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
        void write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t slave_addr) const;
        template<typename T, std::size_t Size> void write(T (&output_data)[Size], uint8_t slave_addr) const {write(Size, (uint8_t*)output_data, slave_addr);}

    private:
        static bool AlreadyUseI2C0;
        static bool AlreadyUseI2C1;
        bool _i2c_id;
    };
    bool I2C::AlreadyUseI2C0 = false;
    bool I2C::AlreadyUseI2C1 = false;

    // SPI通信を行います
    class SPI : public Communication
    {
    public:
        // SPIのセットアップ  SPI0とSPI1を使う際にそれぞれ一回だけ呼び出す
        // spi_id : SPI0かSPI1か
        // sck_pin : SPIのSCKピン
        // mosi_pin : SPIのMOSI(TX)ピン
        // miso_pin : SPIのMISO(RX)ピン
        // cs_pins : SPIのCS(SS)ピン 使用するものすべて (波かっこ{}の中にカンマで区切って書く)
        // freq : SPIの転送速度
        SPI(bool spi_id, Pin sck_pin, Pin mosi_pin, Pin miso_pin, std::initializer_list<Pin> cs_pins, uint32_t freq);

        // SPIで送信しながら受信
        // input_data_bytes : 何バイト(文字)読み込むか (省略した場合はinput_dataの長さだけ読み取る)
        // input_data : 受信したデータを保存するための配列
        // cs_gpio : 通信先のデバイスに繋がるCSピンのGPIO番号
        // output_data : データを受信している間に送信するデータ(1バイト)  データを1バイト受信するごとに1回送信する
        void read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t cs_gpio, uint8_t output_data) const;

        // SPIで受信
        // input_data_bytes : 何バイト(文字)読み込むか (省略した場合はinput_dataの長さだけ読み取る)
        // input_data : 受信したデータを保存するための配列
        // cs_gpio : 通信先のデバイスに繋がるCSピンのGPIO番号
        void read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t cs_gpio) const {read(input_data_bytes, input_data, cs_gpio, 0);}
        template<typename T, std::size_t Size> void read(T (&input_data)[Size], uint8_t cs_gpio) const {read(Size, (uint8_t*)input_data, cs_gpio);}

        // SPIで送信
        // output_data_bytes : 何バイト(文字)書き込むか (省略した場合はoutput_dataの長さだけ書き込む)
        // output_data : 送信するデータの配列
        // cs_gpio : 通信先のデバイスに繋がるCSピンのGPIO番号
        void write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t cs_gpio) const;
        template<typename T, std::size_t Size> void write(T (&output_data)[Size], uint8_t cs_gpio) const {write(Size, (uint8_t*)output_data, cs_gpio);}

        // メモリから読み込み
        // memory_addr : 相手のデバイスの何番地のメモリーからデータを読み込むか
        // input_data_bytes : 何バイト(文字)データを読み込むか (省略した場合は，input_dataの長さだけ読み込む)
        // input_data : 受信したデータを保存するための配列
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        // SPIの場合はメモリーアドレスの8ビット目を自動で0に置き換えます
        void read_mem(uint8_t memory_addr, std::size_t input_data_bytes, uint8_t* input_data, uint8_t select_device = DeviceNotSelected) const
        {
            memory_addr |= 0b10000000;
            this->write(1, &memory_addr, select_device);
            this->read(input_data_bytes, input_data, select_device);
        }

        // メモリに書き込み
        // memory_addr : 相手のデバイスの何番地のメモリーにデータを書き込むか
        // output_data_bytes : 何バイト(文字)データを書き込むか (省略した場合は，output_dataの長さだけ書き込む)
        // output_data : 送信するデータの配列
        // select_device : I2Cではスレーブアドレス，SPIではCSピンのGPIO番号，UARTでは省略
        // SPIの場合はメモリーアドレスの8ビット目を自動で1に置き換えます
        void write_mem(uint8_t memory_addr, std::size_t output_data_bytes, uint8_t* output_data, uint8_t select_device = DeviceNotSelected) const
        {
            memory_addr &= 0b01111111;
            this->write(1, &memory_addr, select_device);
            this->write(output_data_bytes, output_data, select_device);
        }

    private:
        static bool AlreadyUseSPI0;
        static bool AlreadyUseSPI1;
        bool _spi_id;
    };
    bool SPI::AlreadyUseSPI0 = false;
    bool SPI::AlreadyUseSPI1 = false;

    // UART通信を行います
    class UART : public Communication
    {
    public:
        // UARTのセットアップ  UART0とUART1を使う際にそれぞれ一回だけ呼び出す
        // uart_id : UART0かuart1か
        // tx_gpio : UARTのTXピン
        // rx_gpio : UARTのRXピン
        // freq : UARTの転送速度
        UART(bool uart_id, Pin tx_gin, Pin rx_pin, uint32_t freq);

        // UARTで受信
        // input_data_bytes : 何バイト(文字)読み込むか (省略した場合はinput_dataの長さだけ読み取る)
        // input_data : 受信したデータを保存するための配列
        // 引数No_Useは，互換性維持のためにUARTでも付けていますが，UARTでは使用しません．
        void read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t No_Use = DeviceNotSelected) const;
        template<typename T, std::size_t Size> void read(T (&input_data)[Size], uint8_t No_Use = DeviceNotSelected) const {read(Size, (uint8_t*)input_data, No_Use);}

        // UARTで送信
        // output_data_bytes : 何バイト(文字)書き込むか (省略した場合はoutput_dataの長さだけ書き込む)
        // output_data : 送信するデータの配列
        // 引数No_Useは，互換性維持のためにUARTでも付けていますが，UARTでは使用しません．
        void write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t No_Use = DeviceNotSelected) const;
        template<typename T, std::size_t Size> void write(T (&output_data)[Size], uint8_t No_Use = DeviceNotSelected) const {write(Size, (uint8_t*)output_data, No_Use);}

        // 割り込み処理で受信時に自動でデータを読み込み，dequeに保存
        // max_data_bytes : dequeの大きさが最大で何バイトになるまで，読み取った値をdequeの末尾に追加するかを指定します．dequeの大きさがこの値を超えた場合，先頭のデータから削除されます
        // InputData : !クローバル! 読み取った値を追加するdequeを指定します．このdequeは，グローバル変数である必要があります
        // 同じオブジェクトに対し二回以上このメソッドを使った場合の動作は未定義です
        void set_irq(std::size_t max_data_bytes, std::deque<uint8_t>& InputData) const;

    private:
        static bool AlreadyUseUART0;
        static bool AlreadyUseUART1;
        bool _uart_id;
    };
    bool UART::AlreadyUseUART0 = false;
    bool UART::AlreadyUseUART1 = false;

    // PWM(パルス幅変調)を行います
    class PWM : Noncopyable
    {
    public:
        // PWMのセットアップ
        // out_pin : PWMで出力するためのピン
        // [freq] : 設定する周波数 (実際に設定される周波数は少しずれる)
        // [duty] : ピンの出力(ピンの出力が1になっている割合)
        PWM(Pin out_pin, uint32_t freq = 0xffff, float duty = 0.0F);

        // 周波数を設定 (実際に設定される周波数は入力した値から最大で6.25%ずれる)
        // freq : 周波数 (Hz)
        void freq(uint32_t freq_);

        // 周波数を確認
        // 戻り値 : 周波数 (Hz)
        uint32_t freq() const;

        // ピンの出力レベル(ピンの出力が1になっている割合)を設定
        // duty : 出力レベル (0.0 ~ 1.0)
        void level(float duty);

        // ピンの出力レベルを確認
        // 戻り値 : 出力レベル(duty) (0.0 ~ 1.0)
        float level() const;

        // PWMのオン/オフを切り替える
        // on : オンにするときtrue
        void enable(bool on) const;
    private:
        static const uint32_t SysClock = 125E6;  // PWMの基準として使うシステムクロックの周波数 (Hz)
        enum class Channel  // チャンネル (1つのスライスに2つのピンが対応しているため，そのどちらを指すかを識別する値)
        {
            A,
            B
        };

        uint8_t _slice;  // スライス (PWMにおいてピンを識別する番号)
        Channel _channel;  // チャンネル (1つのスライスに2つのピンが対応しているため，そのどちらを指すかを識別する値)
        uint32_t _freq;  // 周波数 (Hz) 最大125M Hz
        float _duty;  // ピンの出力  ピンの出力が1になっている割合
        uint16_t _wrap;  // 分解能 (詳しくは下記の資料へ)
        float _clk_div;  // 分周比 (詳しくは下記の資料へ)
    };
    // PWMクラスは以下の資料を参考にして作成しました
    // https://lipoyang.hatenablog.com/entry/2021/12/12/201432
    // https://rikei-tawamure.com/entry/2021/02/08/213335


    /**************************************************/
    /**********************モーター*********************/
    /**************************************************/

    // 単一モーターの制御
    class Motor1 : Noncopyable
    {
    public:
        // 単一のモーターをセットアップ
        // in1_pin : モータードライバのIN1ピン
        // in2_pin : モータードライバのIN2ピン
        // [freq] : 周波数 (省略時:0xffff)
        // 左右のモーターを扱う場合はMotor2クラスを使用してください
        Motor1(Pin in1_pin, Pin in2_pin, uint32_t freq = 0xffff);

        // モーターを動かす
        // speed : モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        void run(float speed);

        // ブレーキをかける
        void brake();
    private:
        PWM _in1_pwm;
        PWM _in2_pwm;
    };
    // Motor1クラスは以下の資料を参考にして作成しました
    // https://hellobreak.net/raspberry-pi-pico-dc-motor/

    // 左右のモーターの制御
    class Motor2 : Noncopyable
    {
        Motor1 _left_motor;
        Motor1 _right_motor;
    public:
        // 左右のモーターをセットアップ
        // l_in1_pin : 左のモーターのIN1ピン
        // l_in2_pin : 左のモーターのIN2ピン
        // r_in1_pin : 右のモーターのIN1ピン
        // r_in2_pin : 右のモーターのIN2ピン
        // [freq] : 周波数 (省略時:0xffff)
        // 左右のモーターを扱う場合はMotor2クラスを使用してください
        Motor2(Pin l_in1_pin, Pin l_in2_pin, Pin r_in1_pin, Pin r_in2_pin, uint32_t freq = 0xffff):
            _left_motor(l_in1_pin, l_in2_pin, freq),
            _right_motor(r_in1_pin, r_in2_pin, freq) {}

        // 左右のモーターを同時に動かす
        // left_speed : 左モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        // right_speed : 右モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        void run(float left_speed, float right_speed)
        {
            _left_motor.run(left_speed);
            _right_motor.run(right_speed);
        }

        // まっすぐ進む
        // speed : モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        void straight(float speed) {run(speed, speed);}

        // 左に曲がる
        // speed : モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        void left(float speed) {run(0.0F, speed);}

        // 右に曲がる
        // speed : モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
        void right(float speed) {run(speed, 0.0F);}

        // ブレーキをかける
        void brake()
        {
            _left_motor.brake();
            _right_motor.brake();
        }
    };


    /**************************************************/
    /************************記録***********************/
    /**************************************************/

    // SDカードへの書き込み，読み込み
    // 未実装
    class SD
    {
    public:
        void write(std::string message);  // 未実装
        // read();
    };

    // pico本体のFlashメモリへの書き込み，読み込み
    // 未実装
    class Flash
    {
    public:
        void write(std::string message);  // 未実装
        // read();
    };
}

#endif  // SC_PROJECT_RP_PICO_PICO_SDK_SC_SC_HPP_