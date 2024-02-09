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
            // NO input method for now
            if (!is_dlab_set()) {  // Unimplemented
                break;
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

        case PIO_PORT_COM1_IIR_FCR:  // Unimplemented
        case PIO_PORT_COM1_LCR:  // Unimplemented
        case PIO_PORT_COM1_MCR:  // Unimplemented
            break;

        case PIO_PORT_COM1_LSR:
            data_ptr[0] = COM1_REG_LCR_EDHR | COM1_REG_LCR_ETHR;
            // IF INPUT THEN data_ptr[0] = 0x01
            break;

        case PIO_PORT_COM1_MSR:  // Unimplemented
        case PIO_PORT_COM1_SR:  // Unimplemented
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
                IER = data_ptr[0] & COM1_REG_IER_UNUSED_MASK;
                if (IER) {
                    if ((r = vm->flapIRQLine(COM1_IRQ)))
                        return r;
                }
            } else {               // DLH
                DLH = data_ptr[0];
            }
            break;

        case PIO_PORT_COM1_IIR_FCR:  // Unimplemented
            break;

        case PIO_PORT_COM1_LCR:
            LCR = data_ptr[0];
            break;

        case PIO_PORT_COM1_MCR:
        case PIO_PORT_COM1_LSR:
        case PIO_PORT_COM1_MSR:
        case PIO_PORT_COM1_SR:
            break;

        default:
            return 1;
    }

    return 0;
}

COM1::COM1(VM* vm) : IODev(PIO_PORT_COM1_START, PIO_PORT_COM1_SIZE, vm) {}
