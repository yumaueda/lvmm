#ifndef BOOTLOADER_HPP
#define BOOTLOADER_HPP

#include <cstdint>


#pragma pack(1)
struct bootloader_write_param {
    uint16_t vid_mode;
    uint8_t  type_of_loader;
    uint32_t ramdisk_image;
    uint32_t ramdisk_size;
};


#endif  // BOOTLOADER_HPP
