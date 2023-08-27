#include <stdio.h>

#include "pico/stdlib.h"

void setup() {
    stdio_init_all();

    sleep_ms(1000);
}

void loop() {
    printf("Hello, World!\n");
    sleep_ms(1000);
}

int main() {
    setup();

    while (true) loop();
}