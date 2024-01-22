/*
 *  include/post.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_POST_HPP_
#define INCLUDE_POST_HPP_


#include <cstdint>

#include <iodev.hpp>


constexpr uint16_t PIO_PORT_POST_START = 0x80;
constexpr uint8_t  PIO_PORT_POST_SIZE  = 0xA0;


class Post : public IODev {
 public:
    explicit Post();
    int Read(uint16_t, char*, uint8_t) override;
    int Write(uint16_t, char*, uint8_t) override;
};


#endif  // INCLUDE_POST_HPP_
