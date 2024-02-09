/*
 *  include/iodev.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_IODEV_HPP_
#define INCLUDE_IODEV_HPP_


#include <cstdint>

#include <vm.hpp>


class VM;


class IODev {
 public:
    const uint16_t port;
    const uint8_t size;
    VM *vm;

    explicit IODev(uint16_t port, uint8_t size, VM* vm);

    virtual int Read(uint16_t port, char* data_ptr, uint8_t size) = 0;
    virtual int Write(uint16_t port, char* data_ptr, uint8_t size) = 0;
};


#endif  // INCLUDE_IODEV_HPP_
