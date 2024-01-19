/*
 *  include/pio.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_PIO_HPP_
#define INCLUDE_PIO_HPP_


#include <cstdint>


using PIOHandler = int(*)(uint16_t, char*, uint8_t);

constexpr int PIO_PORT_NUM = 2^16;
constexpr uint16_t PIO_PORT_RESET_GENERATOR = 0xCF9;

int default_pio_handler(uint16_t, char*, uint8_t);
int do_nothing_pio_hanlder(uint16_t, char*, uint8_t);
int reset_generator_handler_out(uint16_t, char* data_ptr, uint8_t);


#endif  // INCLUDE_PIO_HPP_
