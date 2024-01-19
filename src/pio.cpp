/*
 *  src/pio.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cerrno>
#include <cstdint>

#include <pio.hpp>


int default_pio_handler(uint16_t, char*, uint8_t) { return 1; }

int do_nothing_pio_hanlder(uint16_t, char*, uint8_t) { return 0; }

int reset_generator_handler_out(uint16_t, char*, uint8_t) {
    return 1;
}
