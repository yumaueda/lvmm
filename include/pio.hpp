/*
 *  include/pio.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_PIO_HPP_
#define INCLUDE_PIO_HPP_


#include <cstdint>


using PIOHundler = int(*)(uint16_t, void*, uint8_t);

constexpr int PIO_PORT_NUM = 2^16;

int default_pio_hundler(uint16_t, void*, uint8_t);
int do_nothing_pio_hunlder(uint16_t, void*, uint8_t);


#endif  // INCLUDE_PIO_HPP_
