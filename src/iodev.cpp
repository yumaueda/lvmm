/*
 *  src/iodev.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <iodev.hpp>

#include <cstdint>

#include <vm.hpp>


IODev::IODev(uint16_t port, uint8_t size, VM* vm)\
        : port(port), size(size), vm(vm) {}
