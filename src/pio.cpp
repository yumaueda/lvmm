/*
 *  src/pio.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cerrno>
#include <cstdint>

#include <pio.hpp>


int default_pio_hundler(uint16_t, void*, uint8_t) { return -ENOSYS; }
int do_nothing_pio_hunlder(uint16_t, void*, uint8_t) { return 0; }
