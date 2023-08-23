#最低限必要なCMakeのバージョンを設定
cmake_minimum_required(VERSION 3.12)

#SDKの読み込み (プロジェクトに関する設定の前にある必要がある)
include(pico_sdk_import.cmake)

#プロジェクト名，プログラミング言語を設定
project(i2c_slave C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

#SDKを初期化
pico_sdk_init()

#ビルドを実行するファイルを追加
add_executable(i2c_slave slave_s.cpp)

#pico_stdlib（ライブラリ）の読み込み
target_link_libraries(i2c_slave hardware_i2c pico_i2c_slave pico_stdlib)

#USB出力を有効にし，UART出力を無効にする
pico_enable_stdio_usb(i2c_slave 1)
pico_enable_stdio_uart(i2c_slave 0)

#map/bin/hex/uf2などのファイルを追加で出力する
pico_add_extra_outputs(i2c_slave)