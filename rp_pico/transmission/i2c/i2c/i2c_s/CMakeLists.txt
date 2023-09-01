#最低限必要なCMakeのバージョンを設定
cmake_minimum_required(VERSION 3.12)

#SDKの読み込み (プロジェクトに関する設定の前にある必要がある)
include(pico_sdk_import.cmake)

#プロジェクト名，プログラミング言語を設定
project(i2c_master_write C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#警告レベルを上げる
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endif()

#SDKを初期化
pico_sdk_init()

#ビルドを実行するファイルを追加
add_executable(i2c_master_write i2c_master_write_s.cpp i2c_s.cpp)

#pico_stdlib（ライブラリ）の読み込み
target_link_libraries(i2c_master_write pico_stdlib pico_i2c_slave hardware_i2c)

#USB出力を有効にし，UART出力を無効にする
pico_enable_stdio_usb(i2c_master_write 1)
pico_enable_stdio_uart(i2c_master_write 0)

#map/bin/hex/uf2などのファイルを追加で出力する
pico_add_extra_outputs(i2c_master_write)