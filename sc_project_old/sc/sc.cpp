#include "sc.hpp"

// Can Sat でよく使うセンサやモータードライバを簡単に使用するためのライブラリです．
namespace sc
{
    /**************************************************/
    /***********************ログ***********************/
    /**************************************************/

    /***** class Error *****/

    // エラーを記録し，標準エラー出力に出力します
    // FILE は＿FILE＿，LINEは＿LINE＿としてください． (自動でファイル名と行番号に置き換わります)
    // message : 出力したいエラーメッセージ (自動で改行)
    Error::Error(const std::string& FILE, int LINE, const std::string& message) noexcept:
        _message(message)
    {
        try
        {
            _message = "<<ERROR>>  FILE : " + std::string(FILE) + "  LINE : " + std::to_string(LINE) + "\n           MESSAGE : " + _message + "\n";  // 出力する形式に変形
            std::cerr << _message << std::endl;  // cerrでエラーとして出力

            save_log(_message);  // エラーをログデータに記録 (外部で定義してください)
        }
        catch (const std::exception& e)
        {
            std::cerr << "<<ERROR>>  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n           MESSAGE : Error logging failed.   " << e.what() << std::endl;
        }  // エラー：エラーログの記録に失敗しました
        // catch(...) {std::cerr << "<<ERROR>>  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n           MESSAGE : Error logging failed." << std::endl;}  // エラー：エラーログの記録に失敗しました
    }

    // ログを記録し，標準出力に出力します
    // 末尾に改行が自動で追加されます
    void inline log(const std::string& message) noexcept
    {
        try
        {
            std::cout << message << std::endl;  // 出力
            save_log(message + '\n');  // 保存
        }
        catch(const std::exception& e) {Error(__FILE__, __LINE__, "Failed to log.", e.what());}  // エラー：ログの記録に失敗しました
        catch(...) {Error(__FILE__, __LINE__, "Failed to log.");}  // エラー：ログの記録に失敗しました
    }


    /**************************************************/
    /*****************測定値および変換******************/
    /**************************************************/

    static constexpr double EarthG = 6.67259E-11;  // 万有引力定数 (m^3 s^-2 kg^-1)
    static constexpr double Earth_g = 9.7803278;  // 赤道における正規重力 (m s^-2) (zero-frequency tide system)
    static constexpr double EarthR = 6378136.62;  // 地球の赤道半径 (m) (zero-frequency tide system)
    static constexpr double EarthF = 298.25642;  // 地球楕円体の逆扁平率 (zero-frequency tide system)
    static constexpr double EarthGM = 3.986004418E14;  // 大気を含めた地心引力定数 (m^3 s^-2) (万有引力定数×地球の質量)

    // エラー値が含まれているかを判断
    // values : エラー値か確かめる値  {1.0, 2.0, 3.0}のように複数の値を入力
    inline bool is_error(std::initializer_list<Measurement> values)
    {
        for (Measurement value : values)
        {
            if (value == ErrorValue)
    return true;
        }
        return false;
    }

    static Pressure AltitudePressure0 = 1013.0;  // 標高を計算する際の基準点の気圧
    static Temperature AltitudeTemperature0 = 25.0;  // 標高を計算する際の基準点の気温
    static Altitude AltitudeAltitude0 = 0.0;  // 標高を計算する際の基準点の標高

    // 標高の基準点をセット
    // pressure0 : 標高の基準点の気圧
    // [temperature0] : 標高の基準点の気温 (省略時:25.0)
    // [altitude0] : 標高の基準点の標高 (省略時:0.0)
    void set_altitude0(const Pressure& pressure0, const Temperature& temperature0 = 25.0, const Altitude& altitude0 = 0.0)
    {
        if (is_error({pressure0, temperature0, altitude0})) throw Error(__FILE__, __LINE__, "Error value cannot be set as the reference point for elevation");  // エラー値を標高の基準点に設定することはできません
        AltitudePressure0 = pressure0;
        AltitudeTemperature0 = temperature0;
        AltitudeAltitude0 = altitude0;
    }

    // 気温と気圧から標高を計算
    // pressure : 測定した気圧
    // temperature : 測定した気温
    // [pressure0] : 基準点の気圧  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude(const Pressure& pressure, const Temperature& temperature, const Pressure& pressure0 = AltitudePressure0, const Altitude& altitude0 = AltitudeAltitude0)
    {
        if (is_error({pressure, temperature, pressure0, altitude0})) return Altitude(ErrorValue);
        return altitude0 + ((temperature + 273.15) / 0.0065) * (std::pow((pressure0 / pressure), (1.0 / 5.257)) - 1.0);
    }

    // 気圧から標高を計算
    // pressure : 測定した気圧
    // [pressure0] : 基準点の気圧  省略可
    // [temperature0] : 基準点の気温  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude(const Pressure& pressure, const Pressure& pressure0 = AltitudePressure0, const Temperature& temperature0 = AltitudeTemperature0, const Altitude& altitude0 = AltitudeAltitude0)
    {
        if (is_error({pressure, pressure0, temperature0, altitude0})) return Altitude(ErrorValue);
        return altitude0 + ((temperature0 + 273.15) / 0.0065) * (1.0 - pow((pressure / pressure0), (1.0 / 5.257)));
    }

    // 気温から標高を計算
    // ！非推奨！精度が悪いです
    // temperature : 測定した気温
    // [temperature0] : 基準点の気温  省略可
    // [altitude0] : 基準点の標高  省略可
    Altitude to_altitude(const Temperature& temperature, const Temperature& temperature0 = AltitudeTemperature0, const Altitude& altitude0 = AltitudeAltitude0) noexcept
    {
        if (is_error({temperature, temperature0, altitude0})) return Altitude(ErrorValue);
        return altitude0 + (temperature0 - temperature) / 0.0065;
    }

    // 重力加速度から標高を計算
    // ！非推奨！精度が悪いです
    // gravity : 測定した重力加速度
    // [gravity0] : 基準点の重力加速度の大きさ (省略時:赤道における正規重力)
    // [altitude0] : 基準点の標高 (省略時: 0m)
    Altitude to_altitude(const GravityAcceleration& gravity, const Magnitude& gravity0 = Earth_g, const Altitude& altitude0 = 0.0) noexcept
    {
        if (is_error({gravity.x, gravity.y, gravity.z, gravity0})) return Altitude(ErrorValue);
        if (to_magnitude(gravity) == 0.0) return DBL_MAX;  // ゼロ除算防止
        if (gravity0 == 0.0) return DBL_MIN;  // ゼロ除算防止
        return sqrt(EarthGM) * (sqrt(1.0 / to_magnitude(gravity)) - sqrt(1.0 / gravity0)) + altitude0;
    }

    // 重力加速度から標高を計算
    // ！非推奨！精度が悪いです
    // gravity : 測定した重力加速度
    // latitude : 測定した地点の緯度
    Altitude to_altitude(const GravityAcceleration& gravity, const Latitude& latitude) noexcept
    {
        if (is_error({gravity.x, gravity.y, gravity.z, latitude})) return Altitude(ErrorValue);
        double lat_s = sin(to_rad(latitude));
        Magnitude gravity0 = 9.7803267715 * (1.0 + 0.001931851353*lat_s*lat_s) / sqrt(1.0 - 0.00669438002290*lat_s*lat_s);  // IGSN71に基づく正規重力値
        return to_altitude(gravity, gravity0, 0.0);
    }

    static Latitude PositionLatitude0 = +35.0;
    static Longitude PositionLongitude0 = +140.0;
    static Altitude PositionAltitude0 = 0.0;
    // XYZ直交座標の原点をセット
    // latitude0 : 原点の緯度
    // longitude0 : 原点の経度
    // [altitude0] : 原点の標高 (省略時:0.0)
    void set_position0(const Latitude& latitude0, const Longitude& longitude0, const Altitude& altitude0 = 0.0)
    {
        if (is_error({latitude0, longitude0, altitude0})) throw Error(__FILE__, __LINE__, "Error value cannot be set to the coordinates of the origin");  // エラー値を原点の座標に設定することはできません
        PositionLatitude0 = latitude0;
        PositionLongitude0 = longitude0;
        PositionAltitude0 = altitude0;
    }

    static constexpr double m0 = 0.999999;  // 平面直交座標系のX軸上における縮尺係数
    static constexpr double n = 1.0 / (2.0*EarthF - 1.0);  // 緯度経度 変換用
    static constexpr double A[6] = {1.0 + n*n/4.0 + n*n*n*n/64.0, -3.0/2.0*(n - n*n*n/8.0 - n*n*n*n*n/64.0), 15.0/16.0*(n*n - n*n*n*n/4.0), -(35.0/48.0)*(n*n*n - (5.0/16.0)*n*n*n*n*n), (315.0/512.0)*n*n*n*n, -(693.0/1280.0)*n*n*n*n*n};  // 緯度経度 変換用
    static constexpr double alpha[5] = {1.0/2.0*n - 2.0/3.0*n*n + 5.0/16.0*n*n*n + 41.0/180.0*n*n*n*n - 127.0/288.0*n*n*n*n*n, 13.0/48.0*n*n - 3.0/5.0*n*n*n + 557.0/1440.0*n*n*n*n + 281.0/630.0*n*n*n*n*n, 61.0/240.0*n*n*n - 103.0/140.0*n*n*n*n + 15061.0/26880.0*n*n*n*n*n, 49561.0/161280*n*n*n*n - 179.0/168.0*n*n*n*n*n, 34729.0/80640.0*n*n*n*n*n};  // 緯度経度 変換用
    static const double tt = 2.0 * sqrt(n) / (1.0+n);  // 緯度経度 変換用
    // 緯度 経度 標高をXYZ直交座標へ変換
    // latitude : 緯度
    // longitude : 経度
    // [altitude] : 標高 (省略時:0.0)
    // [latitude0] : 原点の緯度
    // [longitude0] : 原点の経度
    // [altitude0] : 原点の標高
    A_Position to_position(const Latitude& latitude, const Longitude& longitude, const Altitude& altitude = 0.0, const Latitude& latitude0 = PositionLatitude0, const Longitude& longitude0 = PositionLongitude0, const Altitude& altitude0 = PositionAltitude0) noexcept
    {
        if (is_error({latitude, longitude, altitude, latitude0, longitude0, altitude0})) return A_Position(ErrorValue);
        A_Position output_position;
        double lat0_rad = to_rad(latitude0);
        double lat_s = sin(to_rad(latitude));
        double t = sinh(atanh(lat_s) - tt * atanh(tt * lat_s));
        double d_lon_rad = to_rad(longitude - longitude0);
        double xi = atan(t / cos(d_lon_rad));
        double eta = atanh(sin(d_lon_rad) / sqrt(1 + t*t));
        double A_ = m0*EarthR/(1.0+n)*A[0];
        output_position.y = A_*(xi + alpha[0]*sin(2.0*xi)*cosh(2.0*eta) + alpha[1]*sin(4.0*xi)*cosh(4.0*eta) + alpha[2]*sin(6.0*xi)*cosh(6.0*eta) + alpha[3]*sin(8.0*xi)*cosh(8.0*eta) + alpha[4]*sin(10.0*xi)*cosh(10.0*eta)) - (m0*EarthR / (1.0+n) * (A[0]*lat0_rad + A[1]*sin(2.0*lat0_rad) + A[2]*sin(4.0*lat0_rad) + A[3]*sin(6.0*lat0_rad) + A[4]*sin(8.0*lat0_rad) + A[5]*sin(10.0*lat0_rad)));
        output_position.x = A_*(eta + alpha[0]*cos(2.0*xi)*sinh(2.0*eta) + alpha[1]*cos(4.0*xi)*sinh(4.0*eta) + alpha[2]*cos(6.0*xi)*sinh(6.0*eta) + alpha[3]*cos(8.0*xi)*sinh(8.0*eta) + alpha[4]*cos(10.0*xi)*sinh(10.0*eta));
        output_position.z = altitude - altitude0;
        return output_position;

        // この関数の作成にあたり以下の資料を参考にしました
        // https://vldb.gsi.go.jp/sokuchi/surveycalc/surveycalc/algorithm/bl2xy/bl2xy.htm
    }

    // 緯度 経度 標高をXYZ直交座標へ変換 (高速，簡易版)
    // latitude : 緯度
    // longitude : 経度
    // [altitude] : 標高 (省略時:0.0)
    // [latitude0] : 原点の緯度
    // [longitude0] : 原点の経度
    // [altitude0] : 原点の標高
    A_Position to_position_simple(const Latitude& latitude, const Longitude& longitude, const Altitude& altitude, const Latitude& latitude0, const Longitude& longitude0, const Altitude& altitude0) noexcept
    {
        if (is_error({latitude, longitude, altitude, latitude0, longitude0, altitude0})) return A_Position(ErrorValue);
        A_Position output_position;
        output_position.y = EarthR * sin(to_rad(latitude - latitude0));
        output_position.x = EarthR * sin(to_rad(longitude - longitude0));
        output_position.z = altitude - altitude0;
        return output_position;
    }

    // 2点の座標から直線距離を計算
    // position : XYZ直交座標での位置  1つ目
    // position0 : XYZ直交座標での位置  2つ目
    Distance to_distance(const A_Position& position, const A_Position& position0) noexcept
    {
        if (is_error({position.x, position.y, position.z, position0.x, position0.y, position0.z})) return Distance(ErrorValue);
        double dx = position.x - position0.x;
        double dy = position.y - position0.y;
        double dz = position.z - position0.z;
        return sqrt(dx*dx + dy*dy + dz*dz);
    }

    // 2点の緯度と経度から，地球楕円体表面上の精密な距離を計算
    // latitude : 1つ目の位置の 緯度
    // longitude : 1つ目の位置の 経度
    // [latitude0] : 2つ目の位置の 緯度
    // [longitude0] : 2つ目の位置の 経度
    Distance to_distance(const Latitude& latitude, const Longitude& longitude, const Latitude& latitude0 = PositionLatitude0, const Longitude& longitude0 = PositionLongitude0) noexcept;

    // 2点の緯度と経度から，大円距離を計算 (地球を真球と考えた時の表面上の距離)
    // latitude : 1つ目の位置の 緯度
    // longitude : 1つ目の位置の 経度
    // [latitude0] : 2つ目の位置の 緯度
    // [longitude0] : 2つ目の位置の 経度
    Distance to_distance_simple(const Latitude& latitude, const Longitude& longitude, const Latitude& latitude0 = PositionLatitude0, const Longitude& longitude0 = PositionLongitude0) noexcept
    {
        return EarthR * acos(sin(to_rad(latitude))*sin(to_rad(latitude0)) + cos(to_rad(latitude))*cos(to_rad(latitude0))*cos(to_rad(longitude - longitude0)));

        // この関数の作成にあたり以下の資料を参考にしました
        // https://orsj.org/wp-content/corsj/or60-12/or60_12_701.pdf
    }

    // 2点の座標から方位を計算
    A_Angle to_angle(const A_Position& position, const A_Position& position0)
    {
        // 未実装
    }

    // 2つの方位から相対的な向きを計算
    R_Angle to_r_angle(const A_Angle& angle, const A_Angle& angle0)
    {
        // 未実装
    }

    // 相対的な向きと基準の方位から方位を計算

    /**************************************************/
    /**********************センサ**********************/
    /**************************************************/



    /**************************************************/
    /***********************通信***********************/
    /**************************************************/

    /***** class Pin *****/

    // ピンの設定
    // gpio : ピンのGPIO番号
    // [mode_] : 出力用か，入力用か (省略時:変更しない)
    // [pull_] : ピンのプルアップ・プルダウン設定 (省略時:変更しない)
    Pin::Pin(uint8_t gpio, Mode mode_, Pull pull_):
        _gpio(gpio)
    {
        gpio_init(_gpio);  // ピンを初期化

        mode(mode_);  // モードを設定

        pull(pull_);  // プルアップ・プルダウンを設定
    }

    // ピンを初期化し，再設定
    // [mode_] : 出力用か，入力用か
    // [pull_] : ピンのプルアップ・プルダウン設定
    void Pin::init(Mode mode_, Pull pull_)
    {

        mode(mode_);  // モードを設定

        pull(pull_);  // プルアップ・プルダウンを設定
    }

    // ピンのGPIO番号を取得
    uint8_t Pin::gpio() const
    {
        return _gpio;
    }

    // ピンの入出力モードを取得
    Pin::Mode Pin::mode() const
    {
        if (gpio_get_dir(_gpio))
        {
    return Mode::OUT;
        } else {
    return Mode::IN;
        }
    }

    // ピンの入出力モードを設定
    // mode_ : 出力用か，入力用か
    void Pin::mode(Mode mode_) const
    {
        switch (mode_)
        {
            case Mode::MODE_NO_CHANGE:
        break;
            case Mode::IN:
            {
                gpio_set_dir(_gpio, GPIO_IN);
        break;
            }
            case Mode::OUT:
            {
                gpio_set_dir(_gpio, GPIO_OUT);
        break;
            }
        }
    }

    // ピンのプルアップ・プルダウン状態を取得
    Pin::Pull Pin::pull() const
    {
        if (gpio_is_pulled_up(_gpio))
    return Pull::PULL_UP;
        else if (gpio_is_pulled_down(_gpio))
    return Pull::PULL_DOWN;
        else
    return Pull::PULL_NONE;
    }

    // ピンのプルアップ・プルダウン状態を設定
    // pull_ : ピンのプルアップ・プルダウン設定
    void Pin::pull(Pull pull_) const
    {
        switch (pull_)
        {
            case Pull::PULL_NO_CHANGE:
            {
        break;
            }
            case Pull::PULL_NONE:
            {
                gpio_disable_pulls(_gpio);
        break;
            }
            case Pull::PULL_UP:
            {
                gpio_pull_up(_gpio);
        break;
            }
            case Pull::PULL_DOWN:
            {
                gpio_pull_down(_gpio);
        break;
            }
        }
    }

    // 出力用のピンの出力レベルを1にする
    // 出力モードになっていない場合の動作は未定義です
    void Pin::high() const
    {
        gpio_put(_gpio, 1);
    }

    // 出力用のピンの出力レベルを0にする
    // 出力モードになっていない場合の動作は未定義です
    void Pin::low() const
    {
        gpio_put(_gpio, 0);
    }

    // 入力用ピンの値を取得
    // 入力モードになっていない場合の動作は未定義です
    bool Pin::level() const
    {
        return gpio_get(_gpio);
    }

    // 出力用ピンの出力レベルを設定
    // level_ : 出力レベル  0か1
    // 出力モードになっていない場合の動作は未定義です
    void Pin::level(bool level_) const
    {
        gpio_put(_gpio, level_);
    }

    /***** class Communication *****/

    // 改行までデータを読み込む
    // input_data_bytes : 最大で何バイト(文字)データを読み込むか (省略した場合は，最大でinput_dataの長さだけ読み込む)
    // input_data : 受信したデータを保存するための配列
    // select_device : I2Cではスレーブアドレス，UARTではCSピンのGPIO番号，UARTでは省略
    void Communication::read_line(std::size_t input_data_bytes, uint8_t* input_data, uint8_t select_device) const
    {
        if (!input_data_bytes)
    return;
        do
        {
            read(1U, input_data, select_device);
            if (*(++input_data) == static_cast<uint8_t>('\n'))
    return;
        } while (--input_data_bytes);
    }

    /***** class I2C *****/

    // I2Cのセットアップ  I2C0とI2C1を使う際にそれぞれ一回だけ呼び出す
    // i2c_id : I2C0かi2c1か
    // scl_pin : I2CのSCLのピン (ピン番号のみ指定したもの)
    // sda_pin : I2CのSDAのピン (ピン番号のみ指定したもの)
    // freq : I2Cの転送速度
    I2C::I2C(bool i2c_id, Pin scl_pin, Pin sda_pin, const uint32_t& freq):
        _i2c_id(i2c_id)
    {
        if (_i2c_id)
        {
            if (AlreadyUseI2C1) throw Error(__FILE__, __LINE__, "I2C1 cannot be initialized twice");  // I2C1を二回初期化することはできません
            i2c_init(i2c1, freq);  // I2Cの初期化
            AlreadyUseI2C1 = true;
        } else {
            if (AlreadyUseI2C1) throw Error(__FILE__, __LINE__, "I2C0 cannot be initialized twice");  // I2C0を二回初期化することはできません
            i2c_init(i2c0, freq);  // I2Cの初期化
            AlreadyUseI2C0 = true;
        }

        gpio_set_function(scl_pin.gpio(), GPIO_FUNC_I2C);  // GPIOピンの有効化
        gpio_pull_up(scl_pin.gpio());
        gpio_set_function(sda_pin.gpio(), GPIO_FUNC_I2C);  // GPIOピンの有効化
        gpio_pull_up(sda_pin.gpio());
        
        sleep_ms(10);  // 要検証
    }

    // I2Cで受信
    // input_data_bytes : 何バイト(文字)読み込むか (省略した場合はinput_dataの長さだけ読み取る)
    // input_data : 受信したデータを保存するための配列
    // slave_addr : 通信先のデバイスのCSピンのGPIO番号
    void I2C::read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t slave_addr) const
    {
        if (_i2c_id) {
            i2c_read_blocking(i2c1, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        } else {
            i2c_read_blocking(i2c0, slave_addr, input_data, input_data_bytes, false);  // データを受信  3番目の引数は受信したデータを保存する配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        }

        sleep_ms(10);  // 要検証
    }

    // I2Cで送信
    // output_data_bytes : 何バイト(文字)書き込むか (省略した場合はoutput_dataの長さだけ書き込む)
    // output_data : 送信するデータの配列
    // slave_addr : 通信先のデバイスのスレーブアドレス (どのデバイスにデータを書き込むか) 通常は8~119の間を使用する  7bit
    void I2C::write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t slave_addr) const
    {
        if (_i2c_id) {
            i2c_write_blocking(i2c1, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        } else {
            i2c_write_blocking(i2c0, slave_addr, output_data, output_data_bytes, false);  // データを送信  3番目の引数は，送信するデータの配列の先頭へのポインタ  5番目の引数は，停止信号を送らず次の通信まで他のデバイスに割り込ませないか
        }

        sleep_ms(10);  // 要検証
    }

    /***** class SPI *****/

    // SPIのセットアップ  SPI0とSPI1を使う際にそれぞれ一回だけ呼び出す
    // spi_id : SPI0かSPI1か
    // sck_pin : SPIのSCKピン
    // mosi_pin : SPIのMOSI(TX)ピン
    // miso_pin : SPIのMISO(RX)ピン
    // cs_pins : SPIのCS(SS)ピン 使用するものすべて (波かっこ{}の中にカンマで区切って書く)
    // freq : SPIの転送速度
    SPI::SPI(bool spi_id, Pin sck_pin, Pin mosi_pin, Pin miso_pin, std::initializer_list<Pin> cs_pins, uint32_t freq):
        _spi_id(spi_id)
    {
        if (_spi_id)
        {
            if (AlreadyUseSPI1) throw Error(__FILE__, __LINE__, "SPI1 cannot be initialized twice");  // SPI1を二回初期化することはできません
            spi_init(spi1, freq);  // SPIの初期化
            AlreadyUseSPI1 = true;
        } else {
            if (AlreadyUseSPI1) throw Error(__FILE__, __LINE__, "SPI0 cannot be initialized twice");  // SPI0を二回初期化することはできません
            spi_init(spi0, freq);  // SPIの初期化
            AlreadyUseSPI0 = true;
        }

        gpio_set_function(miso_pin.gpio(), GPIO_FUNC_SPI);
        gpio_set_function(sck_pin.gpio(), GPIO_FUNC_SPI);
        gpio_set_function(mosi_pin.gpio(), GPIO_FUNC_SPI);

        for (Pin cs_pin : cs_pins)
        {
            gpio_init(cs_pin.gpio());
            gpio_set_dir(cs_pin.gpio(), GPIO_OUT);
            gpio_put(cs_pin.gpio(), 1);
        }
    }

    // SPIで受信
    // [input_data_bytes] : 何バイト(文字)読み込むか (省略時:input_dataの長さだけ読み取る)
    // input_data : 受信したデータを保存するための配列
    // cs_gpio : 通信先のデバイスに繋がるCSピンのGPIO番号
    // output_data : データを受信している間に送信するデータ(1バイト)  データを1バイト受信するごとに1回送信する
    void SPI::read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t cs_gpio, uint8_t output_data) const
    {
        gpio_put(cs_gpio, 0);  // CSピンを選択
        if (_spi_id) {
            spi_read_blocking(spi1, output_data, (uint8_t*)input_data, input_data_bytes);  // データを受信  2番目の引数はデータを受信している間に送信するデータ  3番目の引数は受信したデータを保存する配列の先頭へのポインタ
        } else {
            spi_read_blocking(spi0, output_data, (uint8_t*)input_data, input_data_bytes);  // データを受信  2番目の引数はデータを受信している間に送信するデータ  3番目の引数は受信したデータを保存する配列の先頭へのポインタ
        }
        gpio_put(cs_gpio, 1);  // CSピンの選択を解除

        sleep_ms(10);  // 要検証
    }

    // SPIで送信
    // [output_data_bytes] : 何バイト(文字)書き込むか (省略時:output_dataの長さだけ書き込む)
    // output_data : 送信するデータの配列
    // cs_gpio : 通信先のデバイスに繋がるCSピンのGPIO番号
    void SPI::write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t cs_gpio) const
    {
        gpio_put(cs_gpio, 0);  // CSピンを選択
        if (_spi_id) {
            spi_write_blocking(spi1, (uint8_t*)output_data, output_data_bytes);  // データを送信
        } else {
            spi_write_blocking(spi0, (uint8_t*)output_data, output_data_bytes);  // データを送信
        }
        gpio_put(cs_gpio, 1);  // CSピンの選択を解除

        sleep_ms(10);  // 要検証
    }

    /***** class UART *****/

    // UARTのセットアップ  UART0とUART1を使う際にそれぞれ一回だけ呼び出す
    // uart_id : UART0かuart1か
    // tx_gpio : UARTのTXピン
    // rx_gpio : UARTのRXピン
    // freq : UARTの転送速度
    UART::UART(bool uart_id, Pin tx_pin, Pin rx_pin, uint32_t freq):
        _uart_id(uart_id)
    {
        if (_uart_id)
        {
            if (AlreadyUseUART1) throw Error(__FILE__, __LINE__, "UART1 cannot be initialized twice");  // UART1を二回初期化することはできません
            uart_init(uart1, freq);  // UARTの初期化
            AlreadyUseUART1 = true;
        } else {
            if (AlreadyUseUART1) throw Error(__FILE__, __LINE__, "UART0 cannot be initialized twice");  // UART0を二回初期化することはできません
            uart_init(uart0, freq);  // UARTの初期化
            AlreadyUseUART0 = true;
        }

        gpio_set_function(tx_pin.gpio(), GPIO_FUNC_UART);
        gpio_set_function(rx_pin.gpio(), GPIO_FUNC_UART);
    }

    // UARTで受信
    // [input_data_bytes] : 何バイト(文字)読み込むか (省略時:input_dataの長さだけ読み取る)
    // input_data : 受信したデータを保存するための配列
    // 引数No_Useは，互換性維持のためにUARTでも付けていますが，UARTでは使用しません．
    void UART::read(std::size_t input_data_bytes, uint8_t *input_data, uint8_t No_Use) const
    {
        if (_uart_id) {
            uart_read_blocking(uart1, input_data, input_data_bytes);  // データを受信  2番目の引数はデータを受信している間に送信するデータ  3番目の引数は受信したデータを保存する配列の先頭へのポインタ
        } else {
            uart_read_blocking(uart0, input_data, input_data_bytes);  // データを受信  2番目の引数はデータを受信している間に送信するデータ  3番目の引数は受信したデータを保存する配列の先頭へのポインタ
        }

        sleep_ms(10);  // 要検証
    }

    // UARTで送信
    // [output_data_bytes] : 何バイト(文字)書き込むか (省略時:output_dataの長さだけ書き込む)
    // output_data : 送信するデータの配列
    // 引数No_Useは，互換性維持のためにUARTでも付けていますが，UARTでは使用しません．
    void UART::write(std::size_t output_data_bytes, uint8_t *output_data, uint8_t No_Use) const
    {
        if (_uart_id) {
            uart_write_blocking(uart1, (uint8_t*)output_data, output_data_bytes);  // データを送信
        } else {
            uart_write_blocking(uart0, (uint8_t*)output_data, output_data_bytes);  // データを送信
        }

        sleep_ms(10);  // 要検証
    }

    static std::deque<uint8_t> DefaultDeque(0);
    // 割り込み処理で読み取ったデータを保存するための変数を返す関数
    inline static std::deque<uint8_t>& uart0_data(std::deque<uint8_t>& dq = DefaultDeque)
    {
        static std::deque<uint8_t>& uart0_dq = dq;
        return uart0_dq;
    }
    static std::size_t Uart0MaxLen = 100;  // 最大で何バイトまで保存するか
    // 割り込み処理で実行する関数
    void uart0_handler()
    {
        while (uart_is_readable(uart0))
        {
            uart0_data().push_back(uart_getc(uart0));
            uart0_data().pop_front();
        }
        if (uart0_data().size() != Uart0MaxLen)
        {
            uart0_data().resize(Uart0MaxLen, 0U);
        }
    }

    // 割り込み処理で読み取ったデータを保存するための変数を返す関数
    inline static std::deque<uint8_t>& uart1_data(std::deque<uint8_t>& dq = DefaultDeque)
    {
        static std::deque<uint8_t>& uart1_dq = dq;
        return uart1_dq;
    }
    static std::size_t Uart1MaxLen = 100;  // 最大で何バイトまで保存するか
    // 割り込み処理で実行する関数
    void uart1_handler()
    {
        while (uart_is_readable(uart1))
        {
            uart1_data().push_back(uart_getc(uart1));
            uart1_data().pop_front();
        }
        if (uart1_data().size() != Uart1MaxLen)
        {
            uart1_data().resize(Uart1MaxLen, 0U);
        }
    }

    // 割り込み処理で受信時に自動でデータを読み込み，dequeに保存
    // max_data_bytes : dequeの大きさが最大で何バイトになるまで，読み取った値をdequeの末尾に追加するかを指定します．dequeの大きさがこの値を超えた場合，先頭のデータから削除されます
    // InputData : !クローバル! 読み取った値を追加するdequeを指定します．このdequeは，グローバル変数である必要があります
    // 同じオブジェクトに対し二回以上このメソッドを使った場合の動作は未定義です
    void UART::set_irq(std::size_t max_data_bytes, std::deque<uint8_t>& InputData) const
    {
        InputData.resize(max_data_bytes, 0U);
        if (_uart_id)
        {
            Uart1MaxLen = max_data_bytes;
            uart_set_hw_flow(uart1, false, false);  // フロー制御(受信準備が終わるまで送信しないで待つ機能)を無効にします
            uart_set_format(uart1, 8, 1, UART_PARITY_NONE);  // UART通信の設定をします
            uart_set_fifo_enabled(uart1, false);  // FIFO(受信したデータを一時的に保管する機能)をオフにし，1文字ずつ受信する
            irq_set_exclusive_handler(UART1_IRQ, uart1_handler);  // 割り込み処理で実行する関数をセット
            irq_set_enabled(UART1_IRQ, true);  // 割り込み処理を有効にする
        } else {
            Uart0MaxLen = max_data_bytes;
            uart_set_hw_flow(uart0, false, false);  // フロー制御(受信準備が終わるまで送信しないで待つ機能)を無効にします
            uart_set_format(uart0, 8, 1, UART_PARITY_NONE);  // UART通信の設定をします
            uart_set_fifo_enabled(uart0, false);  // FIFO(受信したデータを一時的に保管する機能)をオフにし，1文字ずつ受信する
            irq_set_exclusive_handler(UART0_IRQ, uart0_handler);  // 割り込み処理で実行する関数をセット
            irq_set_enabled(UART0_IRQ, true);  // 割り込み処理を有効にする
        }
    }

    /***** class PWM *****/

    // PWMのセットアップ
    // out_pin : PWMで出力するためのピン
    // freq : 設定する周波数 (実際に設定される周波数は少しずれる)
    // duty : ピンの出力(ピンの出力が1になっている割合)
    PWM::PWM(Pin out_pin, uint32_t freq_, float duty)
    {
        if (out_pin.mode() == Pin::IN) out_pin.init(Pin::OUT, Pin::PULL_NONE);
        gpio_set_function(out_pin.gpio(), GPIO_FUNC_PWM);

        _slice = pwm_gpio_to_slice_num(out_pin.gpio());
        _channel = (pwm_gpio_to_channel(out_pin.gpio()) == 1 ? Channel::B : Channel::A);

        freq(freq_);  // 周波数を設定 (実際に設定される周波数は入力した値から最大で6.25%ずれる)
        level(duty);  // ピンの出力(ピンの出力が1になっている割合)を設定

        pwm_set_phase_correct(_slice, false);  // 三角波にするか
        pwm_set_output_polarity(_slice, false, false);  // チャンネルAとBの出力値の0と1を反転させるか

        enable(true);  // PWMをオンにする
    }

    // 周波数を設定 (実際に設定される周波数は入力した値から最大で6.25%ずれる) (8以上125M以下)
    // freq : 周波数 (Hz)
    void PWM::freq(uint32_t freq_)
    {
        if (freq_ < 8) freq_ = 8;  // 小さすぎる値を修正
        if (freq_ > 125E6) freq_ = 125E6;  // 大きすぎる値を修正

        if ((SysClock / not0(freq_)) > (0xffff + 1))  // 分解能はできる限り高くなるように設定
        {
            _wrap = 0xffff;
        } else {
            _wrap = (SysClock / not0(freq_)) - 1;
        }
        _clk_div = float(SysClock) / not0(float(freq_ * (_wrap + 1)));  // 分解能の値をもとに分周比の値を計算

        _clk_div = float(uint16_t(_clk_div * 16.0F)) / 16.0F;  // 分周比を 1/16 の倍数に直す
        _freq = SysClock/not0(float((_wrap + 1)*_clk_div));  // 上で決定した分解能と分周比を使った場合に，実際に設定される周波数を計算

        pwm_set_wrap(_slice, _wrap);  // 分解能を設定 (詳しくは下記の資料へ)
        pwm_set_clkdiv(_slice, _clk_div);  // 分周比を設定 (詳しくは下記の資料へ)
    }

    // 周波数を確認
    // 戻り値 : 周波数 (Hz)
    uint32_t PWM::freq() const
    {
        return _freq;
    }

    // ピンの出力レベル(ピンの出力が1になっている割合)を設定
    // duty : 出力レベル (0.0 ~ 1.0)
    void PWM::level(float duty)
    {
        if (duty < 0.0F) duty = 0.0F;  // 小さすぎる値を修正
        if (duty > 1.0F) duty = 1.0F;  // 大きすぎる値を修正
        _duty = duty;
        pwm_set_chan_level(_slice, (_channel==Channel::A ? PWM_CHAN_A : PWM_CHAN_B), (unsigned)(_wrap * _duty));
    }

    // ピンの出力レベルを確認
    // 戻り値 : 出力レベル(duty) (0.0 ~ 1.0)
    float PWM::level() const
    {
        return _duty;
    }

    // PWMのオン/オフを切り替える
    // on : オンにするときtrue
    void PWM::enable(bool on) const
    {
        pwm_set_enabled(_slice, on);
    }
    // PWMクラスは以下の資料を参考にして作成しました
    // https://lipoyang.hatenablog.com/entry/2021/12/12/201432
    // https://rikei-tawamure.com/entry/2021/02/08/213335

    /**************************************************/
    /**********************モーター*********************/
    /**************************************************/

    /***** class Motor1 *****/

    // 単一のモーターをセットアップ
    // pin : モーターのピン
    // [freq] : 周波数 (省略時:0xffff)
    // 左右のモーターを扱う場合はMotor2クラスを使用してください
    Motor1::Motor1(Pin in1_pin, Pin in2_pin, uint32_t freq):
        _in1_pwm(in1_pin, 0xffff, 0.0),
        _in2_pwm(in2_pin, 0xffff, 0.0) {}

    // モーターを動かす
    // speed : モーターの出力  -1.0以上+1.0以下の値  負の値のとき逆回転
    void Motor1::run(float speed)
    {
        if (speed < -1.0F) speed = -1.0F;  // 小さすぎる値を修正
        if (speed > +1.0F) speed = +1.0F;  // 大きすぎる値を修正

        if (speed >= 0.0F)
        {
            // 正回転するとき
            _in1_pwm.level(speed);
            _in2_pwm.level(0.0F);
        } else {
            // 逆回転するとき
            _in1_pwm.level(0.0F);
            _in2_pwm.level(speed);
        }
    }

    // ブレーキをかける
    void Motor1::brake()
    {
        _in1_pwm.level(1.0F);
        _in2_pwm.level(1.0F);
    }     


    /**************************************************/
    /************************記録***********************/
    /**************************************************/

    // SDカードへの書き込み
    void SD::write(std::string message)
    {
        // 未実装
    }

    // pico本体のFlashメモリへの書き込み
    void Flash::write(std::string message)
    {
        // 未実装
    }
}
