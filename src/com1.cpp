/*
 *  src/com1.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


// TODO: It should be generalized and implemented in serial, not COM1


#include <com1.hpp>

#include <cstdint>
#include <unistd.h>

#include <iodev.hpp>
#include <vm.hpp>


bool COM1::is_dlab_set() {
    return LCR&COM1_REG_LCR_DLAB;
}

int COM1::Read(uint16_t port, char* data_ptr, uint8_t) {
    switch (port) {
        case PIO_PORT_COM1_THR_RBR_DLL:
            if (!is_dlab_set()) {  // RBR (no RX yet)
                data_ptr[0] = 0;
            } else {               // DLL
                data_ptr[0] = COM1_REG_DLL_9600;
            }
            break;

        case PIO_PORT_COM1_IER_DLH:
            if (!is_dlab_set()) {  // IER
                data_ptr[0] = IER;
            } else {               // DLH
                data_ptr[0] = COM1_REG_DLH_9600;
            }
            break;

        case PIO_PORT_COM1_IIR_FCR:
            // Top two bits report FIFO state (16550A when enabled);
            // bottom bit set means "no interrupt pending".
            data_ptr[0] = COM1_REG_IIR_NO_INT_PEND;
            if (FCR & COM1_REG_FCR_ENABLE)
                data_ptr[0] |= COM1_REG_IIR_FIFO_16550A;
            break;

        case PIO_PORT_COM1_LCR:
            data_ptr[0] = LCR;
            break;

        case PIO_PORT_COM1_MCR:
            data_ptr[0] = MCR;
            break;

        case PIO_PORT_COM1_LSR:
            data_ptr[0] = COM1_REG_LSR_TEMT | COM1_REG_LSR_THRE;
            break;

        case PIO_PORT_COM1_MSR:
            if (MCR & COM1_REG_MCR_LOOP) {
                // Loopback: upper nibble reflects MCR control bits.
                data_ptr[0] = 0;
                if (MCR & COM1_REG_MCR_DTR)  data_ptr[0] |= COM1_REG_MSR_DSR;
                if (MCR & COM1_REG_MCR_RTS)  data_ptr[0] |= COM1_REG_MSR_CTS;
                if (MCR & COM1_REG_MCR_OUT1) data_ptr[0] |= COM1_REG_MSR_RI;
                if (MCR & COM1_REG_MCR_OUT2) data_ptr[0] |= COM1_REG_MSR_DCD;
            } else {
                // No real peer; report CTS/DSR/DCD asserted so flow control passes.
                data_ptr[0] = COM1_REG_MSR_CTS
                            | COM1_REG_MSR_DSR
                            | COM1_REG_MSR_DCD;
            }
            break;

        case PIO_PORT_COM1_SR:
            data_ptr[0] = SR;
            break;

        default:
            return 1;
    }
    return 0;
}

int COM1::Write(uint16_t port, char* data_ptr, uint8_t) {
    int r;

    switch (port) {
        case PIO_PORT_COM1_THR_RBR_DLL:
            if (!is_dlab_set()) {  // THR
                // In loopback mode the byte should land in RBR, not the host
                // serial line. RX path isn't implemented yet, so just drop it.
                if (MCR & COM1_REG_MCR_LOOP)
                    break;
                r = write(STDOUT_FILENO, data_ptr, 1);
                if (r == -1) {
                    perror(("COM1::" + std::string(__func__) +
                                ": write").c_str());
                    return -errno;
                }
            } else {               // DLL
                DLL = data_ptr[0];
            }
            break;

        case PIO_PORT_COM1_IER_DLH:
            if (!is_dlab_set()) {  // IER
                IER = data_ptr[0] & COM1_REG_IER_WRITABLE_MASK;
            } else {               // DLH
                DLH = data_ptr[0];
            }
            break;

        case PIO_PORT_COM1_IIR_FCR:
            FCR = data_ptr[0];
            break;

        case PIO_PORT_COM1_LCR:
            LCR = data_ptr[0];
            break;

        case PIO_PORT_COM1_MCR:
            MCR = data_ptr[0];
            break;

        case PIO_PORT_COM1_LSR:
        case PIO_PORT_COM1_MSR:
            // Read-only in real hardware; ignore writes.
            break;

        case PIO_PORT_COM1_SR:
            SR = data_ptr[0];
            break;

        default:
            return 1;
    }

    return 0;
}

COM1::COM1(VM* vm) : IODev(PIO_PORT_COM1_START, PIO_PORT_COM1_SIZE, vm) {}
