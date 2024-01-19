/*
 *  src/pio.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cstdint>
#include <iostream>

#include <pio.hpp>


int default_pio_handler(uint16_t, char*, uint8_t) {
    return 1;
}

int do_nothing_pio_hanlder(uint16_t, char*, uint8_t) {
    return 0;
}

int reset_generator_handler_out(uint16_t, char* data_ptr, uint8_t) {
    std::cerr << "reset generator: output to port 0xCF9 detected: "
        << data_ptr[0] << std::endl;
    return 1;
}
