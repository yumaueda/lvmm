#ifndef INCLUDE_PCI_HPP_
#define INCLUDE_PCI_HPP_


#include <cstdint>
#include <vector>

constexpr uint16_t PIO_PORT_PCI_CONFIG_ADDR = 0x0CF8;
constexpr uint16_t PIO_PORT_PCI_CONFIG_DATA = 0x0CFC;

constexpr uint16_t PIO_PORT_PCI_CSAM2_START = 0xC000;
constexpr uint16_t PIO_PORT_PCI_CSAM2_END   = 0xD000;

constexpr uint8_t  PCI_ADDR_SIZE            = 4;

constexpr uint32_t PCI_ADDR_OFFSET_MASK     = 0x0000'00FC;
constexpr uint32_t PCI_ADDR_FUNC_MASK       = 0x0000'0700;
constexpr uint32_t PCI_ADDR_DEV_MASK        = 0x0000'F800;
constexpr uint32_t PCI_ADDR_BUS_MASK        = 0x00FF'0000;

constexpr uint32_t PCI_ADDR_FUNC_BIT        = 8;
constexpr uint32_t PCI_ADDR_DEV_BIT         = 11;
constexpr uint32_t PCI_ADDR_BUS_BIT         = 16;
constexpr uint32_t PCI_ADDR_ENABLE_BIT      = 31;

constexpr uint32_t PCI_BAR_MMIO             = 0;
constexpr uint32_t PCI_BAR_PIO              = 1;

class PCIDevice {
    virtual int read(uint16_t port, char* data_ptr, uint8_t size)  = 0;
    virtual int write(uint16_t port, char* data_ptr, uint8_t size) = 0;
};

class PCI {
 public:
    uint32_t addr;
    std::vector<PCIDevice> device;

    bool     is_addr_enable();
    // should be inline?
    uint32_t get_offset();
    uint32_t get_func();
    uint32_t get_dev();
    uint32_t get_bus();

    int config_addr_in(uint16_t, char* data_ptr, uint8_t size);
    int config_addr_out(uint16_t, char* data_ptr, uint8_t size);
    //int config_data_in(uint16_t port, char* data_ptr, uint8_t size);
    //int config_data_out(uint16_t port, char* data_ptr, uint8_t size);
};


#endif  // INCLUDE_PCI_HPP_
