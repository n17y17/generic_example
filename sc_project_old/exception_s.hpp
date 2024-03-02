#ifndef CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_EXCEPTION_S_HPP_
#define CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_EXCEPTION_S_HPP_

#include <unordered_map>
#include <iostream>  // cerr, endl
#include <string>  // string()
#include <chrono>  // 処理を再度行うための時間計測

/*
このファイルでは，プログラム全体のエラーを記録，制御する関数を定義しています．
エラーを記録することで，ある関数の外部から，その関数の状態(最近呼び出したときに正常に動作したか)を知ることができるようにしています
*/

// ゼロ徐算防止
template<class T> inline T not0(T n) {return (n ? n : (T)1);}

class Error : public std::exception
{
    std::string what_;
public:
    Error(std::string file, unsigned line, std::string message) :
        what_(message)
    {
        try
        {
            message = "<<ERROR>>  FILE : " + std::string(file) + "  LINE : " + std::to_string(line) + "\n           MESSAGE : " + message;  // 出力する形式に変形
            std::cerr << message << std::endl;  // cerrでエラーとして出力

            // ここで，エラーをログデータに記録（未実装）
        }
        catch(const std::exception& ee) {std::cerr << "!!ERROR  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!         MESSAGE : " << ee.what() << std::endl;}
        catch(...) {std::cerr << "!!ERROR  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!       MESSAGE : Unknown error occurred" << std::endl;}
    }
    const char* what() const noexcept override
    {
        std::cout << "what" << std::endl;
        return what_.c_str();
    }
};

void Log(std::string message) noexcept
{
    try
    {
        std::cout << message << std::endl;

        // ここでログデータにも記録する（未実装）
    }
    catch(const std::exception& e)
    {
        Error(__FILE__, __LINE__, "Failed to record log data");  // ログデータの記録に失敗しました
    }
    catch(...)
    {
        Error(__FILE__, __LINE__, "Failed to record log data");  // ログデータの記録に失敗しました
    }
}

// class Log
// {
//     std::string message_;
// public:
//     Log(std::string message):
//         message_(message)
//     {
//         std::cout << message << std::endl;

//         // ここでログデータにも記録する（未実装）
//     }
// }

// /*!
//  \brief エラーを記録します
// 記録をするだけなので，動作を停止することはありません．
//  \param file ＿FILE＿ と書いてください
//  \param line ＿LINE＿ と書いてください
// (コンパイル時に自動でファイル名と行番号に置き換わります)
//  \param message エラーの概要
// */
// void SaveError(const char* file, unsigned line, std::string message)
// {
//     try
//     {
//         message = "\nMMMMMMMMM\n!!ERROR!!  FILE : " + std::string(file) + "  LINE : " + std::to_string(line) + "\nWWWWWWWWW  MESSAGE : " + message + "\n";  // 出力するメッセージをstring型の変数に入れる
//         std::cerr << message << std::endl;  // cerrでエラーとして出力

//         // ここで，エラーをログデータに記録（未実装）
//     }
//     catch(const std::exception& e) {std::cerr << "!!ERROR  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!       MESSAGE : " << e.what() << std::endl;}
//     catch(...) {std::cerr << "!!ERROR  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!       MESSAGE : Unknown error occurred" << std::endl;}
// }

// /*!
//  \brief エラーを記録します
// 記録をするだけなので，動作を停止することはありません．
//  \param file ＿FILE＿ と書いてください
//  \param line ＿LINE＿ と書いてください
// (コンパイル時に自動でファイル名と行番号に置き換わります)
//  \param e 投げられた例外，または　std::runtime_error("メッセージ")　の形式の式
// */
// void SaveError(const char* file, unsigned line, const std::exception& e = std::runtime_error("Unknown error occurred"))
// {
//     SaveError(file, line, e.what());
// }

// /*!
//  \brief 失敗を記録します
// 記録をするだけなので，動作を停止することはありません．
//  \param file ＿FILE＿ と書いてください
//  \param line ＿LINE＿ と書いてください
// (コンパイル時に自動でファイル名と行番号に置き換わります)
//  \param message 失敗した処理の概要
// */
// void SaveFailure(const char* file, unsigned line, std::string message)
// {
//     try
//     {
//         message = "<<ERROR>>  FILE : " + std::string(file) + "  LINE : " + std::to_string(line) + "\n           MESSAGE : " + message;  // 出力する形式に変形
//         std::cerr << message << std::endl;  // cerrでエラーとして出力

//         // ここで，エラーをログデータに記録（未実装）
//     }
//     catch(const std::exception& ee) {std::cerr << "!!FAILURE  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!         MESSAGE : " << ee.what() << std::endl;}
//     catch (...) {std::cerr << "!!ERROR  FILE : " << __FILE__ << "  LINE : " << __LINE__ << "/n!!       MESSAGE : Unknown error occurred" << std::endl;}
// }

// /*!
//  \brief 失敗を記録します
// 記録をするだけなので，動作を停止することはありません．
//  \param file ＿FILE＿ と書いてください
//  \param line ＿LINE＿ と書いてください
// (コンパイル時に自動でファイル名と行番号に置き換わります)
//  \param e 投げられた例外
// */
// void SaveFailure(const char* file, unsigned line, const std::exception& e = std::runtime_error("Unknown failure occurred"))
// {
//     SaveFailure(file, line, e.what());
// }

// /*
// 処理が成功したか
// kSuccess : 成功しました
// kFailure : 失敗しました
// kError : エラーが発生しました
// */
// enum Result
// {
//     kSuccess = -4,  // 成功しました
//     kFailure,  // 失敗しました
//     kError,  // 実行時エラーが発生しました
//     kSeriousError  // 致命的なエラー(状態をkStoppedに固定)
// };

// /*
// 動作状況
// kWorking : 正常に稼働中
// kLittle : 動かない時も多い
// kStopped : ほとんど動かない
// */
// enum class State
// {
//     kWorking = -4,  // 正常に稼働中
//     kLittle,  // 動かない時も多い
//     kStopped,  // ほとんど動かない
// };

// static const uint16_t kMaxStatus = 65535;  // 処理の成功状況を記録する際の最大値
// static const uint16_t kDefaultStatus = 60000;  // 処理の成功状況を記録する際の初期値
// static const uint16_t kMinStatus = 0;  // 処理の成功状況を記録する際の最小値
// static std::unordered_map<std::string, std::pair<bool, uint16_t>> kStatus;  // 処理の成功状況を記録する変数  処理に成功すると増加し，失敗すると減少する
// static auto kStatusIncrementTimerStart = std::chrono::steady_clock::now();  // 一定時間経過ごとに処理の成功状況の値を増加させるためのタイマーです
// static const float kStatusIncrementTimeDiff = 30;  // 何秒に一回，成功状況の値を増加さえるのか

// // 一定時間の経過を確認したとき，処理の成功状況の値を増加させます
// inline static void IncrementStatus()
// {
//     std::chrono::duration<float> diff = std::chrono::steady_clock::now() - kStatusIncrementTimerStart;
//     if (diff.count() > kStatusIncrementTimeDiff)
//     {
//         kStatusIncrementTimerStart = std::chrono::steady_clock::now();
//         for (std::pair<const std::string, std::pair<bool, uint16_t>>& status : kStatus)
//         {
//             if (status.second.first) continue;
//             ++status.second.second;
//         }
//     }
// }

// /*!
//  \brief 処理の結果を保存
//  \param result Result型の結果（Result::kSuccess, Result::kFailure, Result::kError）
//  \param process_name 文字列型の処理のID．process_nameは，{ } でくくることで複数個まとめて指定できます
// */
// struct SaveResult
// {
//     SaveResult(Result result, std::string process_name)
//     {
//         try
//         {
//             if (kStatus.count(process_name) == 0) kStatus[process_name] = std::make_pair(false, kDefaultStatus);  // 新たに動作状況の変数を作る場合は，デフォルトの値で初期化
//             if (kStatus[process_name].first) return;  // 動作状況の変更にロックがかかっていた場合，変更しない
//             uint16_t& this_status = kStatus[process_name].second;
//             switch (result)
//             {
//                 case Result::kSuccess:
//                 {
//                     if (this_status <= (kMaxStatus - 1)) this_status += 1;  // 関数の処理に成功したため加算
//                     break;
//                 }
//                 case Result::kFailure:
//                 {
//                     if (this_status >= (kMinStatus + 10)) this_status -= 30;  // 関数の処理に失敗したため減算
//                     break;
//                 }
//                 case Result::kError:
//                 {
//                     if (this_status >= (kMinStatus + 50)) this_status -= 100;  // 関数の処理中にエラーが発生したため大きく減算
//                     break;
//                 }
//                 case Result::kSeriousError:
//                 {
//                     kStatus[process_name].first = true;  // 状態をkStoppedに固定
//                 }
//                 default:
//                 {
//                     SaveError(__FILE__, __LINE__, std::domain_error("A value not defined in Result was given."));
//                 }
//             }
//             if (this_status < kMinStatus) this_status = kMinStatus;
//             if (this_status > kMaxStatus) this_status = kMaxStatus;
//             IncrementStatus();
//         }

//         // エラー時の処理
//         catch(const std::exception& e) {SaveError(__FILE__, __LINE__, e);}
//         catch(...) {SaveError(__FILE__, __LINE__);}
//     }
//     SaveResult(Result result, std::initializer_list<std::string> process_name_s)
//     {
//         try
//         {
//             for (std::string process_neme : process_name_s)
//             {
//                 SaveResult(result, process_neme);
//             }
//         }

//         // エラー時の処理
//         catch(const std::exception& e) {SaveError(__FILE__, __LINE__, e);}
//         catch(...) {SaveError(__FILE__, __LINE__);}
//     }
// };

// static const uint8_t kMinWorkingSuccessRate = 70;  // 何％以上動作しているときに，「動作中」の判定にするか
// static const uint8_t kMinLittleSuccessRate = 30;  // 何％以上動作しているときに，「少しだけ動作中」の判定にするか
// /*!
//  \brief ある処理が，今まで正常に実行されてきたかを返します
//  \param process_name 文字列型の処理のID
// */
// inline State GetStatus(std::string process_name)
// {
//     IncrementStatus();
//     if (kStatus.count(process_name) == 0) kStatus[process_name] = std::make_pair(false, kDefaultStatus);  // 新たに動作状況の変数を作る場合は，デフォルトの値で初期化
//     if (kStatus[process_name].first) return State::kStopped;
//     uint8_t success_rate = (kStatus[process_name].second - kMinStatus) / not0(kMaxStatus - kMinStatus);  // 処理の成功割合 (%)
//     if (success_rate > kMinWorkingSuccessRate) return State::kWorking;
//     else if (success_rate > kMinLittleSuccessRate) return State::kLittle;
//     else return State::kStopped;
// }

#endif  // CANSAT_MAIN_SYSTEM_RP_PICO_PICO_SDK_EXCEPTION_S_HPP_