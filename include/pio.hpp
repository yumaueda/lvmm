/*
 *  include/pio.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_PIO_HPP_
#define INCLUDE_PIO_HPP_


#include <cstdint>
#include <functional>


using PIOHandler = std::function<int(uint16_t, char*, uint8_t)>;


constexpr uint16_t PIO_PORT_NUM             = UINT16_MAX;

// Port number for devices that do not have a dedicated include file
constexpr uint16_t PIO_PORT_ALT_DELAY_START = 0xED;
constexpr uint16_t PIO_PORT_ALT_DELAY_END   = 0xEE;

constexpr uint16_t PIO_PORT_COM4_START      = 0x2E8;
constexpr uint16_t PIO_PORT_COM4_END        = 0x2F0;

constexpr uint16_t PIO_PORT_COM2_START      = 0x2F8;
constexpr uint16_t PIO_PORT_COM2_END        = 0x300;

constexpr uint16_t PIO_PORT_VGA_0_START     = 0x3B0;  // 0x3B4?
constexpr uint16_t PIO_PORT_VGA_0_END       = 0x3BC;  // 0x3B6?
constexpr uint16_t PIO_PORT_VGA_1_START     = 0x3C0;
constexpr uint16_t PIO_PORT_VGA_1_END       = 0x3E0;  // 0x3DB?

constexpr uint16_t PIO_PORT_COM3_START      = 0x3E8;
constexpr uint16_t PIO_PORT_COM3_END        = 0x3F0;

constexpr uint16_t PIO_PORT_COM1_START      = 0x3F8;
constexpr uint16_t PIO_PORT_COM1_END        = 0x400;

constexpr uint16_t PIO_PORT_RST_GEN_START   = 0xCF9;
constexpr uint16_t PIO_PORT_RST_GEN_END     = 0xCFA;

constexpr uint16_t PIO_PORT_UNKNOWN_1_START = 0xCFA;  // ?
constexpr uint16_t PIO_PORT_UNKNOWN_1_END   = 0xCFC;  // ?

constexpr uint16_t PIO_PORT_UNKNOWN_2_START = 0xCFE;  // ?
constexpr uint16_t PIO_PORT_UNKNOWN_2_END   = 0xCFF;  // ?


int default_pio_handler(uint16_t, char*, uint8_t);
int do_nothing_pio_handler(uint16_t, char*, uint8_t);
int reset_generator_handler_out(uint16_t, char* data_ptr, uint8_t);


#endif  // INCLUDE_PIO_HPP_
