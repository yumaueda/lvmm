/*
 *  include/pio.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_PIO_HPP_
#define INCLUDE_PIO_HPP_


#include <cstdint>


using PIOHandler = int(*)(char*, uint8_t);

constexpr int      PIO_PORT_NUM           = 2^16;
constexpr uint16_t PIO_PORT_RST_GEN_START = 0xCF9;
constexpr uint16_t PIO_PORT_RST_GEN_END   = 0xCFA;
constexpr uint16_t PIO_PORT_VGA_0_START   = 0x3B0;  // 0x3B4?
constexpr uint16_t PIO_PORT_VGA_0_END     = 0x3BC;  // 0x3B6?
constexpr uint16_t PIO_PORT_VGA_1_START   = 0x3C0;
constexpr uint16_t PIO_PORT_VGA_1_END     = 0x3E0;  // 0x3DB?

int default_pio_handler(char*, uint8_t);
int do_nothing_pio_handler(char*, uint8_t);
int reset_generator_handler_out(char* data_ptr, uint8_t);


#endif  // INCLUDE_PIO_HPP_
