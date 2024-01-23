/*
 *  src/cmos.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cmos.hpp>

#include <chrono>
#include <cstdint>
#include <ctime>

#include <iodev.hpp>


int CMOS::Read(uint16_t port, char* data_ptr, uint8_t size) {
    if (size != 1) {
        return 1;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t nowc = std::chrono::system_clock::to_time_t(now);
    struct tm nowtm;
    localtime_r(&nowc, &nowtm);

    switch (port) {
        case PIO_PORT_CMOS_INDEX:
            *data_ptr = index;
            break;

        case PIO_PORT_CMOS_DATA:
            switch (index) {
                // tmp
                // still many regs need to be implemented!
                case CMOS_REG_SEC:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_sec));
                    break;

                case CMOS_REG_MIN:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_min));
                    break;

                case CMOS_REG_HOUR:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_hour));
                    break;

                case CMOS_REG_WEEKD:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_wday));
                    break;

                case CMOS_REG_MDAY:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_mday));
                    break;

                case CMOS_REG_MONTH:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_mon));
                    break;

                case CMOS_REG_YEAR:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>(nowtm.tm_year % 100));
                    break;

                case CMOS_REG_STAT_A:
                    *data_ptr = CMOS_REG_STAT_A_AVL
                              | CMOS_REG_STAT_A_DV1
                              | CMOS_REG_STAT_A_RS2
                              | CMOS_REG_STAT_A_RS1;
                    break;

                case CMOS_REG_STAT_B:
                    *data_ptr = CMOS_REG_STAT_B_24H;
                    break;

                case CMOS_REG_STAT_C:
                    *data_ptr = 0;
                    break;

                case CMOS_REG_STAT_D:
                    *data_ptr = CMOS_REG_STAT_D_VRB;
                    break;

                case CMOS_REG_CENTURY:
                    *data_ptr = bin_to_bcd(
                            static_cast<uint8_t>((nowtm.tm_year+1900)/100));
                    break;

                default:
                    *data_ptr = data[index];
            }

            // R/W 0x71 default the selected reg to 0x0D
            index = CMOS_REG_STAT_D;
            break;
        default:
            return 1;
    }

    return 0;
}

int CMOS::Write(uint16_t port, char* data_ptr, uint8_t size) {
    if (size != 1) {
        return 1;
    }

    switch (port) {
        // Ignore an NMI disabling bit
        case PIO_PORT_CMOS_INDEX:
            index = *data_ptr & CMOS_INDEX_MASK;
            break;
        case PIO_PORT_CMOS_DATA:
            if (index == CMOS_REG_SHUTDOWN && *data_ptr == 0) {
                index = CMOS_REG_STAT_D;
                break;
            }
            data[index] = *data_ptr;
            // R/W 0x71 default the selected reg to 0x0D
            index = CMOS_REG_STAT_D;
            break;
        default:
            return 1;
    }

    return 0;
}

CMOS::CMOS() : IODev(PIO_PORT_CMOS_START, PIO_PORT_CMOS_SIZE) {}
