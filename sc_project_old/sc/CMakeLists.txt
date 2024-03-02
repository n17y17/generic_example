# ビルドを実行するファイルを追加
add_executable(SC sc.cpp bme280.cpp)

# pico_stdlib（ライブラリ）の読み込み
target_link_libraries(SC pico_stdlib hardware_gpio hardware_i2c hardware_spi hardware_uart hardware_pwm)

# USB出力を有効にし，UART出力を無効にする
pico_enable_stdio_usb(SC 1)
pico_enable_stdio_uart(SC 0)

# map/bin/hex/uf2などのファイルを追加で出力する
pico_add_extra_outputs(SC)