#ifndef CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_BME280_S_HPP_
#define CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_BME280_S_HPP_

#include "communication_s.hpp"

class Bme280 : public Sensor
{
public:
    /*! \brief BME280のセットアップを行います
        \param commu 使用する通信のオブジェクト(I2c型またはSpi型)へのポインタ
        \param slave_addr BME280のスレーブアドレス*/
    Bme280(Communication*, uint8_t);

    // 気温，気圧，湿度 を計測
    void Measure(Measurement&);

    // 標高を計算するための基準点をセット
    void SetOrigin();

    /* チップIDを取得することで接続を確認
       trueならBME280またはBMP280に接続できている
       センサがBME280であればチップIDは必ず0x60になるが，
       センサがその下位製品であるBMP280であればチップIDは0x58になる（湿度を測定できない）*/
    bool CheckConnection() noexcept;

private:
    /*  I2c型かSpi型のオブジェクトへのポインタ
        BME280はI2CとSPIの両方で通信ができる．
        Communication型はI2c型とSpi型の両方のオブジェクトを入れられる*/
    Communication* commu_;

    /*  通信相手のデバイスを選択するためのアドレス 
        I2C通信の場合のスレーブアドレス
        SPI通信の場合のCS(チップセレクト)ピンのGPIO番号  */
    uint8_t device_select_;

    float altitude0_;  // 標高計算の基準点の高度
    float temperature0_;  // 標高計算の基準点の気温
    float pressure0_;  // 標高計算の基準点の気圧

    int32_t temperature_;  // 受信したデータを一時保管しておくための変数
    int32_t pressure_;  // 受信したデータを一時保管しておくための変数
    int32_t humidity_;  // 受信したデータを一時保管しておくための変数

    // 補正用データ読み取り
    void ReadCompensationData();

    // 測定モードとオーバーサンプリングを設定
    void SetModeAndOverSampling(Mode, OverSampling);

    // 生データ読み取り (人間が使う単位にはなっていない)
    void ReadRaw();

    // 気温データ補正
    void CorrectTemperature(Measurement&);

    // 気圧データ補正
    void CorrectPressure(Measurement&);

    // 湿度データ補正
    void CorrectHumidity(Measurement&);

    // 測定のモード
    enum Mode
    {
        sleep,
        forced,
        normal = 3
    }

    /* オーバーサンプリング
       精度が高まるが，測定に時間がかかるようになる*/
    enum OverSampling
    {
        skipped,  // 測定しない?
        x1,
        x2,
        x4,
        x8,
        x16
    };

    // 測定と測定の間の待機時間の長さ
    enum StanbyTime
    {
        st0_5ms,  // 0.5ms
        st62_5ms,  // 62.5ms
        st125ms,  // 125ms
        st250ms,  // 250ms
        st500ms,  // 500ms
        st1000ms,  // 1000ms
        st10ms,  // 10ms
        st20ms  // 20ms
    };

    /*
    ノイズ除去フィルターの係数
    値が大きいほどノイズが減るが，計測に時間がかかる
    */
    enum Filter{
        filter_off,
        f2,
        f4,
        f8,
        f16
    };

    // SPI使用時に，3線式を使用するか，4線式を使用するか
    enum SpiWireNum
    {
        wire4,
        wire3
    };

    uint16_t dig_T1;  // 気温補正用データ
    int16_t dig_T2, dig_T3;  // 気温補正用データ
    uint16_t dig_P1;  // 気圧補正用データ
    int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P8, dig_P9;  // 気圧補正用データ
    uint8_t dig_H1, dig_H3;  // 湿度補正用データ
    int8_t dig_H6;  // 湿度補正用データ
    int16_t dig_H2, dig_H4, dig_H5;  // 湿度補正用データ

    uint8_t chip_id_;  // チップID
};

#endif  // CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_BME280_S_HPP_