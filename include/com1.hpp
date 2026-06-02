/*
 *  include/com1.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


// TODO: It should be generalized and implemented in serial, not COM1


#ifndef INCLUDE_COM1_HPP_
#define INCLUDE_COM1_HPP_


#include <cstdint>
#include <termios.h>
#include <atomic>
#include <deque>
#include <mutex>
#include <thread>

#include <iodev.hpp>
#include <vm.hpp>

constexpr uint32_t COM1_IRQ = 4;

constexpr uint16_t PIO_PORT_COM1_START       = 0x3F8;
constexpr uint16_t PIO_PORT_COM1_SIZE        = 0x8;

constexpr uint16_t PIO_PORT_COM1_THR_RBR_DLL = PIO_PORT_COM1_START;
constexpr uint16_t PIO_PORT_COM1_IER_DLH     = PIO_PORT_COM1_START + 1;
constexpr uint16_t PIO_PORT_COM1_IIR_FCR     = PIO_PORT_COM1_START + 2;
constexpr uint16_t PIO_PORT_COM1_LCR         = PIO_PORT_COM1_START + 3;
constexpr uint16_t PIO_PORT_COM1_MCR         = PIO_PORT_COM1_START + 4;
constexpr uint16_t PIO_PORT_COM1_LSR         = PIO_PORT_COM1_START + 5;
constexpr uint16_t PIO_PORT_COM1_MSR         = PIO_PORT_COM1_START + 6;
constexpr uint16_t PIO_PORT_COM1_SR          = PIO_PORT_COM1_START + 7;

// Not 16750 comp.
constexpr uint8_t COM1_REG_DLL_9600          = 0x0C;
constexpr uint8_t COM1_REG_IER_WRITABLE_MASK = 0b0000'1111;
constexpr uint8_t COM1_REG_IER_DATA_AVL_MASK = 0b0000'0001;
constexpr uint8_t COM1_REG_DLH_9600          = 0x00;
constexpr uint8_t COM1_REG_LCR_DLAB          = 0b1000'0000;
constexpr uint8_t COM1_REG_LSR_TEMT          = 0b0100'0000;
constexpr uint8_t COM1_REG_LSR_THRE          = 0b0010'0000;
constexpr uint8_t COM1_REG_FCR_ENABLE        = 0b0000'0001;
constexpr uint8_t COM1_REG_IIR_NO_INT_PEND   = 0b0000'0001;
constexpr uint8_t COM1_REG_IIR_FIFO_16550A   = 0b1100'0000;
constexpr uint8_t COM1_REG_MCR_DTR           = 0b0000'0001;
constexpr uint8_t COM1_REG_MCR_RTS           = 0b0000'0010;
constexpr uint8_t COM1_REG_MCR_OUT1          = 0b0000'0100;
constexpr uint8_t COM1_REG_MCR_OUT2          = 0b0000'1000;
constexpr uint8_t COM1_REG_MCR_LOOP          = 0b0001'0000;
constexpr uint8_t COM1_REG_MSR_CTS           = 0b0001'0000;
constexpr uint8_t COM1_REG_MSR_DSR           = 0b0010'0000;
constexpr uint8_t COM1_REG_MSR_RI            = 0b0100'0000;
constexpr uint8_t COM1_REG_MSR_DCD           = 0b1000'0000;


class COM1 : public IODev {
 public:
    explicit COM1(VM* vm);
    ~COM1();
    int Read(uint16_t port, char* data_ptr, uint8_t) override;
    int Write(uint16_t port, char* data_ptr, uint8_t) override;

 private:
    // Registers (port offset/DLAB/RW)
    // - We have all registers in 16750; Most of them do nothing, however.
    // - Baud rate is fixed at 9600
    uint8_t THR{0}, RBR{0}, DLL{0};  // 0/0/_W, 0/0/R_, 0/1/RW
    uint8_t IER{0}, DLH{0};       // 1/0/RW, 1/1/RW
    uint8_t IIR{0}, FCR{0};       // 2/x/R_, 2/x/_W
    uint8_t LCR{0};            // 3/x/RW
    uint8_t MCR{0};            // 4/x/RW
    uint8_t LSR{0};            // 5/x/R
    uint8_t MSR{0};            // 6/x/R
    uint8_t SR{0};             // 7/x/RW

    // RX path: a reader thread pushes host stdin bytes into rx_buf, and
    // COM1::Read drains rx_buf on RBR reads.
    std::mutex rx_mutex;
    std::deque<uint8_t> rx_buf;
    std::thread reader_thread;
    std::atomic<bool> shutting_down{false};
    bool termios_saved{false};
    struct termios saved_termios;

    void readerLoop();

    bool is_dlab_set();
};


#endif  // INCLUDE_COM1_HPP_
