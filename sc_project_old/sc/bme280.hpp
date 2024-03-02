#ifndef SC_PROJECT_RP_PICO_PICO_SDK_SC_BME280_HPP_
#define SC_PROJECT_RP_PICO_PICO_SDK_SC_BME280_HPP_

#include "sc.hpp"

namespace sc
{
    // BME280 (気温，気圧，湿度センサ) の読み取り
    class BME280 : public Sensor
    {
        // BME280のセットアップ (I2CとSPI共通)
        // device_addr : 通信相手のデバイスを選択するためのアドレス
        // i2c_or spi : I2C型かSPI型のオブジェクト (一時オブジェクト不可)
        BME280(uint8_t select_device, Communication& i2c_or_spi);
    public:
        // BME280のセットアップ (I2C)
        // i2c : I2C型のオブジェクト (一時オブジェクト不可)
        // slave_addr : I2Cのスレーブアドレス
        BME280(I2C& i2c, uint8_t slave_addr):
            BME280(slave_addr, i2c) {}

        // BME280のセットアップ (SPI)
        // spi : SPI型のオブジェクト (一時オブジェクト不可)
        // cs_gpio : CSピンのGPIO番号
        BME280(SPI& spi, uint8_t cs_gpio):
            BME280(cs_gpio, spi) {}

        // センサが正常に接続されていることを確認
        // 戻り値 : 正常:true, 異常:false
        bool check_connection() noexcept;

        // 測定を実行
        void measure() noexcept;

        // 測定した気温を返す
        Temperature temperature() const noexcept;

        // 測定した気圧を返す
        Pressure pressure() const noexcept;

        // 測定した湿度を返す
        Humidity humidity() const noexcept;
    private:
        // I2C型かSPI型のオブジェクト
        // BME280はI2CとSPIの両方で通信ができる．
        // Communication型はI2c型とSpi型の両方のオブジェクトを入れられる
        Communication& _i2c_or_spi;

        // 通信相手のデバイスを選択するためのアドレス
        // I2C通信の場合のスレーブアドレス
        // SPI通信の場合のCS(チップセレクト)ピンのGPIO番号
        uint8_t _select_device;

        int32_t _raw_temperature; // 受信したデータを一時保管しておくための変数 (一般的な単位にはなっていない)
        int32_t _raw_pressure;    // 受信したデータを一時保管しておくための変数 (一般的な単位にはなっていない)
        int32_t _raw_humidity;    // 受信したデータを一時保管しておくための変数 (一般的な単位にはなっていない)

        int32_t _conversion_parameter;  // 気温，温度，湿度を補正する際に使用するパラメータ  (気温によって変わる)

        Temperature _temperature;  // 気温 (補正済)
        Pressure _pressure;  // 気圧 (補正済)
        Humidity _humidity;  // 湿度 (補正済)

        // 測定のモード
        enum Mode
        {
            MODE_SLEEP,
            MODE_FORCED,
            MODE_NORMAL = 3
        };

        // オーバーサンプリング
        // 精度が高まるが，測定に時間がかかるようになる
        enum OverSampling
        {
            OSRS_SKIPPED, // 測定しない?
            OSRS_x1,
            OSRS_x2,
            OSRS_x4,
            OSRS_x8,
            OSRS_x16
        };

        // 測定と測定の間の待機時間の長さ
        enum StanbyTime
        {
            ST_0_5ms,  // 0.5ms
            ST_62_5ms, // 62.5ms
            ST_125ms,  // 125ms
            ST_250ms,  // 250ms
            ST_500ms,  // 500ms
            ST_1000ms, // 1000ms
            ST_10ms,   // 10ms
            ST_20ms    // 20ms
        };

        // ノイズ除去フィルターの係数
        // 値が大きいほどノイズが減るが，計測に時間がかかる
        enum Filter
        {
            FILTER_OFF,
            FILTER_2,
            FILTER_4,
            FILTER_8,
            FILTER_16
        };

        // SPI使用時に，3線式を使用するか，4線式を使用するか
        enum SpiWireNum
        {
            SPI_WIRE_4,
            SPI_WIRE_3
        };

        uint16_t dig_T1;  // 気温補正用データ
        int16_t dig_T2, dig_T3;  // 気温補正用データ
        uint16_t dig_P1;  // 気圧補正用データ
        int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;  // 気圧補正用データ
        uint8_t dig_H1, dig_H3;  // 湿度補正用データ
        int8_t dig_H6;  // 湿度補正用データ
        int16_t dig_H2, dig_H4, dig_H5;  // 湿度補正用データ
        
        // 測定方法などを設定
        // [mode] : 測定のモード (省略時:ノーマル)
        // [over_sampling_t] : 気温のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:2回)
        // [over_sampling_p] : 気圧のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:4回)
        // [over_sampling_h] : 湿度のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:1回)
        // [stanby_time] : 測定と測定の間の待機時間の長さ (省略時:125ms)
        // [filter] : ノイズ除去フィルタの係数 (ノイズを減らすが測定時間が伸びる) (省略時:フィルター2)
        // [spi_wire_num] : SPI使用時に，3線式を使用するか，4線式を使用するか (省略時:4線式)
        void set_parameter(Mode mode = MODE_NORMAL, OverSampling over_sampling_t = OSRS_x2, OverSampling over_sampling_p = OSRS_x4, OverSampling over_sampling_h = OSRS_x1, StanbyTime stanby_time = ST_125ms, Filter filter = FILTER_2, SpiWireNum spi_wire_num = SPI_WIRE_4) const;

        // 補正用データ読み取り
        void read_compensation_data();

        // 生データ読み取り (一般的な単位にはなっていない)
        void read_raw();

        // 補正用のパラメータを計算
        void calc_conversion_parameter();

        // 気温データを補正して保存
        void convert_temperature();

        // 気圧データを補正して保存
        void convert_pressure();

        // 湿度データを補正して保存
        void convert_humidity();
    };
    // このクラスの作成にあたり以下の資料を参考にしました
    // https://akizukidenshi.com/download/ds/bosch/BST-BME280_DS001-10.pdf
    // https://qiita.com/nanase/items/f34e03c29410add9c4d0
}

#endif  // SC_PROJECT_RP_PICO_PICO_SDK_SC_BME280_HPP_