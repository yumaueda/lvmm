/*
 *  include/cmos.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_CMOS_HPP_
#define INCLUDE_CMOS_HPP_


#include <cstdint>

#include <iodev.hpp>
#include <vm.hpp>


constexpr uint16_t PIO_PORT_CMOS_START = 0x70;
constexpr uint8_t  PIO_PORT_CMOS_SIZE  = 0x2;
constexpr uint16_t PIO_PORT_CMOS_INDEX = PIO_PORT_CMOS_START;
constexpr uint16_t PIO_PORT_CMOS_DATA  = PIO_PORT_CMOS_START+1;

constexpr uint8_t  CMOS_INDEX_MASK     = 0b0111'1111;
constexpr int      CMOS_MEM_SIZE       = 128;

constexpr uint8_t  CMOS_REG_SEC        = 0x00;
constexpr uint8_t  CMOS_REG_MIN        = 0x02;
constexpr uint8_t  CMOS_REG_HOUR       = 0x04;
constexpr uint8_t  CMOS_REG_WEEKD      = 0x06;
constexpr uint8_t  CMOS_REG_MDAY       = 0x07;
constexpr uint8_t  CMOS_REG_MONTH      = 0x08;
constexpr uint8_t  CMOS_REG_YEAR       = 0x09;
constexpr uint8_t  CMOS_REG_STAT_A     = 0x0A;
constexpr uint8_t  CMOS_REG_STAT_B     = 0x0B;
constexpr uint8_t  CMOS_REG_STAT_C     = 0x0C;
constexpr uint8_t  CMOS_REG_STAT_D     = 0x0D;
constexpr uint8_t  CMOS_REG_DIAGNOSTIC = 0x0E;
constexpr uint8_t  CMOS_REG_SHUTDOWN   = 0x0F;
constexpr uint8_t  CMOS_REG_CENTURY    = 0x32;

constexpr uint8_t  CMOS_REG_STAT_A_AVL = 0 << 7;
constexpr uint8_t  CMOS_REG_STAT_A_DV1 = 0b010 << 4;
constexpr uint8_t  CMOS_REG_STAT_A_RS2 = 1 << 2;
constexpr uint8_t  CMOS_REG_STAT_A_RS1 = 1 << 1;

constexpr uint8_t  CMOS_REG_STAT_B_24H = 1 << 1;

constexpr uint8_t  CMOS_REG_STAT_D_VRB = 1 << 7;

constexpr uint8_t  CMOS_DT_MAX         = 99;


static inline int bin_to_bcd(uint8_t bin) {
    if (bin > CMOS_DT_MAX)
        return 1;
    return ((bin / 10) << 4 | bin % 10);
}


class CMOS : public IODev {
 public:
    explicit CMOS(VM* vm);
    int Read(uint16_t, char*, uint8_t) override;
    int Write(uint16_t port, char*, uint8_t) override;

 private:
    uint8_t index;
    uint8_t data[CMOS_MEM_SIZE] = { 0 };
};


#endif  // INCLUDE_CMOS_HPP_
