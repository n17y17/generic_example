#include "bme280_s.hpp"

// BME280のセットアップ
Bme280::Bme280(Communication* commu, uint8_t device_select):
    commu_(commu),
    device_select_(device_select)
{
    try
    {
        // 接続を確認
        CheckConnection();

        // 補正データを読み込み
        ReadCompensationData();

        // 測定モードと，オーバーサンプリングを設定
        SetModeAndOverSampling();
    }
    catch(const std::exception& e)
    {
        try {ReadCompensationData();}  // エラーの時は補正データだけでも読み取る
        catch{throw Error(__FILE__, __LINE__, "BME280 setup failed")}  // BME280のセットアップに失敗しました
    }
}

// 気温，気圧，湿度 を計測
void Bme280::Measure(Measurement& measurement)
{
    try
    {
        // 生データ読み取り
        ReadRaw();

        // 気温データ補正
        CorrectTemperature(measurement);

        // 気圧データ補正
        CorrectPressure(measurement);

        // 湿度データ補正
        CorrectHumidity(measurement);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

// 生データ読み取り
void Bme280::ReadRaw()
{
    uint8_t read_data[8];
    pressure_ = temperature_ = humidity_ = -1024;  // この数字は要検討
    try
    {
        commu_->ReadMem(devise_select_, &read_data, 8);
        pressure_ = ((uint32_t) read_data[0] << 12) | ((uint32_t) read_data[1] << 4) | (read_data[2] >> 4);
        temperature_ = ((uint32_t) read_data[3] << 12) | ((uint32_t) read_data[4] << 4) | (read_data[5] >> 4);
        humidity_ = ((uint32_t) read_data[6] << 8) | read_data[7];
    }
    catch(const std::exception& e)  // エラー時の処理
    {
        try
        {
            commu_->ReadMem(devise_select_, &read_data, 6);
            pressure_ = ((uint32_t) read_data[0] << 12) | ((uint32_t) read_data[1] << 4) | (read_data[2] >> 4);
            temperature_ = ((uint32_t) read_data[3] << 12) | ((uint32_t) read_data[4] << 4) | (read_data[5] >> 4);
        }
        catch(const std::exception& e)
        {
            throw Error(__FILE__, __LINE__, "Failed to receive from BME280");  // BME280からの受信に失敗しました
        }
        try {
            commu_->ReadMem(devise_select_, &(read_data[6]), 2);
            humidity_ = ((uint32_t) read_data[6] << 8) | read_data[7];
        }
        catch(const std::exception& e)
        {
            Error(__FILE__, __LINE__, "Failed to receive humidity from BME280");  // BME280からの湿度の受信に失敗しました
        }
        
    }

}

void Bme280::CorrectTemperature(Measurement& measurement)
{

}

// チップIDを取得することで接続を確認
bool Bme280::CheckConnection() noexcept
{
    try
    {
        commu_->ReadMem(device_select_, &chip_id_, 1);  // チップIDを読み取り
    }
    catch(const std::exception& e)
    {
        sleep_ms(10);
        try {commu_->ReadMem(device_select_, &chip_id_, 1);}  // エラーの時はもう一度読み取り
        catch(const std::exception& e)
        {
            Error(__FILE__, __LINE__, "Failed to read BMP280 chip ID");  // BME280のチップIDの読み取りに失敗しました
return false;
        }
    }
    
    if (chip_id_ == 0x58)   // チップIDが正しいかを確認
    {
        Error(__FILE__, __LINE__, "BMP280 is connected instead of BME280. Please use the program dedicated to BMP280.");  // BME280の代わりにBMP280が接続されています．BMP280専用のプログラムを使用してください．
return true;
    } else if (chip_id == 0x60) {
        Log("BME280 was successfully connected");  // BME280は正常に接続されました
return true;
    } else {
        Error(__FILE__, __LINE__, "BMP280 connection could not be confirmed");  // BME280の接続が確認できませんでした
return false;
    }
}

// 補正用データ読み取り
void Bme280::ReadCompensationData()
{
    uint8_t compensation_data[26];

    try
    {
        commu_->ReadMem(device_select_, 0x88, compensation_data, 26);
    }
    catch(const std::exception& e)
    {
        sleep_ms(10);
        try {commu_->ReadMem(device_select_, 0x88, compensation_data, 26);}  // エラーのときはもう一度読み取り
        catch {throw Error(__FILE__, __LINE__, "Failed to read compention data")}  // 補正用データの読み取りに失敗しました
    }
    
    dig_T1 = compensation_data[0] | (compensation_data[1] << 8);
    dig_T2 = compensation_data[2] | (compensation_data[3] << 8);
    dig_T3 = compensation_data[4] | (compensation_data[5] << 8);

    dig_P1 = compensation_data[6] | (compensation_data[7] << 8);
    dig_P2 = compensation_data[8] | (compensation_data[9] << 8);
    dig_P3 = compensation_data[10] | (compensation_data[11] << 8);
    dig_P4 = compensation_data[12] | (compensation_data[13] << 8);
    dig_P5 = compensation_data[14] | (compensation_data[15] << 8);
    dig_P6 = compensation_data[16] | (compensation_data[17] << 8);
    dig_P7 = compensation_data[18] | (compensation_data[19] << 8);
    dig_P8 = compensation_data[20] | (compensation_data[21] << 8);
    dig_P9 = compensation_data[22] | (compensation_data[23] << 8);

    dig_H1 = compensation_data[25];

    try
    {
        commu_->ReadMem(device_select_, 0xE1, compensation_data, 8);
    }
    catch(const std::exception& e)
    {
        sleep_ms(10);
        try {commu_->ReadMem(device_select_, 0xE1, compensation_data, 8);}  // エラーのときはもう一度読み取り
        catch {throw Error(__FILE__, __LINE__, "Failed to read compention data")}  // 補正用データの読み取りに失敗しました
    }

    dig_H2 = compensation_data[0] | (compensation_data[1] << 8);
    dig_H3 = (int8_t) compensation_data[2];
    dig_H4 = compensation_data[3] << 4 | (compensation_data[4] & 0xf);
    dig_H5 = (compensation_data[5] >> 4) | (compensation_data[6] << 4);
    dig_H6 = (int8_t) compensation_data[7];
}

// 測定モードと，オーバーサンプリングを設定
void Bme280::SetModeAndOverSampling(Mode mode = 3, OverSampling over_sampling_t = x4, OverSampling over_sampling_p = x4, OverSampling over_sampling_h = x1, StanbyTime stanby_time = st62_5ms, Filter filter = f4, SpiWireNum spi_wire_num = wire4)
{
    try
    {
        commu_->WriteMem(device_select_, 0xF5, (stanby_time << 5) | (filter << 2) | spi_wire_num, 1);  // 待機時間，ノイズカットフィルタ，SPIの線の本数を設定
        commu_->WriteMem(device_select_, 0xF4, (over_sampling_t << 5) | (over_sampling_p << 2) | mode, 1);  // 気温と気圧のオーバーサンプリングと測定モードを設定
        commu_->WriteMem(device_select_, 0xF2, &over_sampling_h, 1);  // 湿度のオーバーサンプリングを設定
    }
    catch(const std::exception& e)
    {
        Error(__FILE__, __LINE__, "Failed to set measurement mode and oversampling");  // 測定モードと，オーバーサンプリングの設定に失敗しました
    }
}

/*
このプログラムを作成するにあたり，以下を参考にしました
https://akizukidenshi.com/download/ds/bosch/BST-BME280_DS001-10.pdf
https://qiita.com/nanase/items/f34e03c29410add9c4d0
*/