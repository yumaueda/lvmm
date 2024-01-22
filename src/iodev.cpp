/*
 *  src/iodev.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <iodev.hpp>

#include <cstdint>


IODev::IODev(uint16_t port, uint8_t size) : port(port), size(size) {}
