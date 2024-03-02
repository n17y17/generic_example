#include "bme280.hpp"

namespace sc
{
    // BME280のセットアップ (I2CとSPI共通)
    // device_addr : 通信相手のデバイスを選択するためのアドレス
    // i2c_or spi : I2C型かSPI型のオブジェクト (一時オブジェクト不可)
    BME280::BME280(uint8_t select_device, Communication& i2c_or_spi):
        _i2c_or_spi(i2c_or_spi),
        _select_device(select_device)
    {
        // 接続を確認
        check_connection();

        // 測定方法などの設定
        set_parameter();

        // 補正用データ読み取り
        read_compensation_data();
    }

    // センサが正常に接続されていることを確認
    // 戻り値 : 正常:true, 異常:false
    bool BME280::check_connection() noexcept
    {
        try
        {
            uint8_t chip_id;
            _i2c_or_spi.read(1, &chip_id, _select_device);  // チップIDを読み取り，接続されているセンサがBME280であるか確認
            switch (chip_id)
            {
                case 0x60:
                {
                    log("BME280 is connected normally"); // BME280は正常に接続されています
    return true;
                }
                case 0x58:
                {
                    log("BMP280 is connected instead of BME280; use the BMP280-specific program.");  // BME280ではなくBMP280が接続されています．BMP280専用のプログラムを使用してください．
    return true;
                }
                default:
                {
                    log("BME280 connection could not be verified");  // BME280の接続が確認できませんでした
    return false;
                }
            }
        }
        catch(const std::exception& e)
        {
            Error(__FILE__, __LINE__, "Error checking BME280 connection", e.what());  // BME280の接続を確認する際にエラーが発生しました
            return false;
        }
    }

    // 測定を実行
    void BME280::measure() noexcept
    {
        try
        {
            // 最初にエラー値を代入
            _temperature = ErrorValue;
            _pressure = ErrorValue;
            _humidity = ErrorValue;

            // 生データ読み取り (一般的な単位にはなっていない)
            read_raw();

            // 補正用のパラメータを計算
            calc_conversion_parameter();

            // 気温データを補正して保存
            convert_temperature();

            // 気圧データを補正して保存
            convert_pressure();

            // 湿度データを補正して保存
            convert_humidity();
        }
        catch(const std::exception& e)
        {
            Error(__FILE__, __LINE__, "Measurement with BME280 failed", e.what());  // BME280での測定に失敗しました
        }
    }

    // 測定した気温を返す
    Temperature BME280::temperature() const noexcept
    {
        return _temperature;
    }

    // 測定した気圧を返す
    Pressure BME280::pressure() const noexcept
    {
        return _pressure;
    }

    // 測定した湿度を返す
    Humidity BME280::humidity() const noexcept
    {
        return _humidity;
    }

    // 測定方法などを設定
    // [mode] : 測定のモード (省略時:ノーマル)
    // [over_sampling_t] : 気温のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:2回)
    // [over_sampling_p] : 気圧のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:4回)
    // [over_sampling_h] : 湿度のオーバーサンプリング (測定の精度を高めるが測定時間が伸びる) (省略時:1回)
    // [stanby_time] : 測定と測定の間の待機時間の長さ (省略時:125ms)
    // [filter] : ノイズ除去フィルタの係数 (ノイズを減らすが測定時間が伸びる) (省略時:フィルター2)
    // [spi_wire_num] : SPI使用時に，3線式を使用するか，4線式を使用するか (省略時:4線式)
    void BME280::set_parameter(Mode mode = MODE_NORMAL, OverSampling over_sampling_t = OSRS_x2, OverSampling over_sampling_p = OSRS_x4, OverSampling over_sampling_h = OSRS_x1, StanbyTime stanby_time = ST_125ms, Filter filter = FILTER_2, SpiWireNum spi_wire_num = SPI_WIRE_4) const
    {
        // メモリに書き込み
        _i2c_or_spi.write_byte_mem(0xf2, over_sampling_h, _select_device);
        _i2c_or_spi.write_byte_mem(0xf4, ((over_sampling_t << 5) | (over_sampling_p << 2) | mode), _select_device);
        _i2c_or_spi.write_byte_mem(0xf5, ((stanby_time << 5) | (filter << 2) | spi_wire_num), _select_device);
    }

    // 補正用データ読み取り
    void BME280::read_compensation_data()
    {
        uint8_t input_data[26];

        _i2c_or_spi.read_mem(0x88, 24, input_data, _select_device);

        dig_T1 = input_data[0] | (input_data[1] << 8);
        dig_T2 = input_data[2] | (input_data[3] << 8);
        dig_T3 = input_data[4] | (input_data[5] << 8);

        dig_P1 = input_data[6] | (input_data[7] << 8);
        dig_P2 = input_data[8] | (input_data[9] << 8);
        dig_P3 = input_data[10] | (input_data[11] << 8);
        dig_P4 = input_data[12] | (input_data[13] << 8);
        dig_P5 = input_data[14] | (input_data[15] << 8);
        dig_P6 = input_data[16] | (input_data[17] << 8);
        dig_P7 = input_data[18] | (input_data[19] << 8);
        dig_P8 = input_data[20] | (input_data[21] << 8);
        dig_P9 = input_data[22] | (input_data[23] << 8);

        dig_H1 = input_data[24];

        _i2c_or_spi.read_mem(0xE1, 8, input_data, _select_device);

        dig_H2 = input_data[0] | (input_data[1] << 8);
        dig_H3 = (int8_t) input_data[2];
        dig_H4 = input_data[3] << 4 | (input_data[4] & 0xf);
        dig_H5 = (input_data[5] >> 4) | (input_data[6] << 4);
        dig_H6 = (int8_t) input_data[7];
    }

    // 生データ読み取り (一般的な単位にはなっていない)
    void BME280::read_raw()
    {
        uint8_t input_data[8];
        _i2c_or_spi.read_mem(0xf7, 8, input_data, _select_device);

        _raw_pressure = ((uint32_t) input_data[0] << 12) | ((uint32_t) input_data[1] << 4) | (input_data[2] >> 4);
        _raw_temperature = ((uint32_t) input_data[3] << 12) | ((uint32_t) input_data[4] << 4) | (input_data[5] >> 4);
        _raw_humidity = (uint32_t) input_data[6] << 8 | input_data[7];
    }

    // 補正用のパラメータを計算
    void BME280::calc_conversion_parameter()
    {
        _conversion_parameter = (((((_raw_temperature >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11) + ((((((_raw_temperature >> 4) - ((int32_t) dig_T1)) * ((_raw_temperature >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3)) >> 14);
    }

    // 気温データを補正して保存
    void BME280::convert_temperature()
    {
        _temperature = ((_conversion_parameter * 5 + 128) >> 8) / 100.0;
    }

    // 気圧データを補正して保存
    void BME280::convert_pressure()
    {
        int32_t var1, var2;
        uint32_t p;
        var1 = (((int32_t) _conversion_parameter) >> 1) - (int32_t) 64000;
        var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) dig_P6);
        var2 = var2 + ((var1 * ((int32_t) dig_P5)) << 1);
        var2 = (var2 >> 2) + (((int32_t) dig_P4) << 16);
        var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) dig_P2) * var1) >> 1)) >> 18;
        var1 = ((((32768 + var1)) * ((int32_t) dig_P1)) >> 15);
        if (var1 == 0)
        {
            _pressure = 0;
            return;
        }

        p = (((uint32_t) (((int32_t) 1048576) - _raw_pressure) - (var2 >> 12))) * 3125;
        if (p < 0x80000000)
            p = (p << 1) / ((uint32_t) var1);
        else
            p = (p / (uint32_t) var1) * 2;

        var1 = (((int32_t) dig_P9) * ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
        var2 = (((int32_t) (p >> 2)) * ((int32_t) dig_P8)) >> 13;
        p = (uint32_t) ((int32_t) p + ((var1 + var2 + dig_P7) >> 4));

        _pressure = p / 100.0;
    }

    // 湿度データを補正して保存
    void BME280::convert_humidity()
    {
        int32_t v_x1_u32r;
        v_x1_u32r = (_conversion_parameter - ((int32_t) 76800));
        v_x1_u32r = (((((_raw_humidity << 14) - (((int32_t) dig_H4) << 20) - (((int32_t) dig_H5) * v_x1_u32r)) +
                    ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t) dig_H6)) >> 10) * (((v_x1_u32r *
                                                                                                    ((int32_t) dig_H3))
                >> 11) + ((int32_t) 32768))) >> 10) + ((int32_t) 2097152)) *
                                                    ((int32_t) dig_H2) + 8192) >> 14));
        v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) dig_H1)) >> 4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

        _humidity = uint32_t(v_x1_u32r >> 12) / 1024.0;
    }
}