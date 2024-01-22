/*
 *  src/pci.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <pci.hpp>

#include <cstdint>


bool PCI::is_addr_enable() {
    return ((addr >> PCI_ADDR_ENABLE_BIT) & 1) == 1;
}

uint32_t PCI::get_offset() {
    return addr & PCI_ADDR_OFFSET_MASK;
}

uint32_t PCI::get_func() {
    return (addr & PCI_ADDR_FUNC_MASK) >> PCI_ADDR_FUNC_BIT;
}

uint32_t PCI::get_dev() {
    return (addr & PCI_ADDR_DEV_MASK) >> PCI_ADDR_DEV_BIT;
}

uint32_t PCI::get_bus() {
    return (addr & PCI_ADDR_BUS_MASK) >> PCI_ADDR_BUS_BIT;
}

int PCI::config_addr_in(uint16_t, char* data_ptr, uint8_t size) {
    uint32_t* addr_ptr = reinterpret_cast<uint32_t*>(data_ptr);
    if (size != PCI_ADDR_SIZE)
        return 1;
    *data_ptr = *addr_ptr;
    return 0;
}

int PCI::config_addr_out(uint16_t, char* data_ptr, uint8_t size) {
    uint32_t* addr_ptr = reinterpret_cast<uint32_t*>(data_ptr);
    if (size != PCI_ADDR_SIZE)
        return 1;
    addr = *addr_ptr;
    return 0;
}

/*
int PCI::config_data_in(uint16_t port, char* data_ptr, uint8_t size) {
    uint32_t offset = get_offset();
    uint32_t func   = get_func();
    uint32_t dev    = get_dev();
    uint32_t bus    = get_bus();

    if (!is_addr_enable())
        return 1;

    if (dev > device.size())
        return 1;

    // ...

    return 0;
}

int PCI::config_data_out(uint16_t, char* data_ptr, uint8_t size) {
    uint32_t offset = get_offset();
    uint32_t func   = get_func();
    uint32_t dev    = get_dev();
    uint32_t bus    = get_bus();

    if (!is_addr_enable())
        return 1;

    if (dev > device.size())
        return 1;

    // ...

    return 0;
}
*/
