#ifndef BOOTLOADER_HPP
#define BOOTLOADER_HPP

#include <cstdint>


#pragma pack(1)
struct bootloader_write_param {
    const uint16_t vid_mode       = 0xffff;
    const uint8_t  type_of_loader = 0xff;
    const uint32_t ramdisk_image  = 0x0f00'0000;
    uint32_t       ramdisk_size   = 0;
};


#endif  // BOOTLOADER_HPP
