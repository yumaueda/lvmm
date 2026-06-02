/*
 *  src/com1.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


// TODO: It should be generalized and implemented in serial, not COM1


#include <com1.hpp>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#include <iodev.hpp>
#include <vm.hpp>


bool COM1::is_dlab_set() {
    return LCR&COM1_REG_LCR_DLAB;
}

int COM1::Read(uint16_t port, char* data_ptr, uint8_t) {
    switch (port) {
        case PIO_PORT_COM1_THR_RBR_DLL:
            if (!is_dlab_set()) {  // RBR
                std::lock_guard<std::mutex> lk(rx_mutex);
                if (!rx_buf.empty()) {
                    data_ptr[0] = static_cast<char>(rx_buf.front());
                    rx_buf.pop_front();
                } else {
                    data_ptr[0] = 0;
                }
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

        case PIO_PORT_COM1_IIR_FCR: {
            // Interrupt source priority (highest first on a 16550):
            //   Receiver data available > Transmitter holding register empty.
            bool rx_pending;
            {
                std::lock_guard<std::mutex> lk(rx_mutex);
                rx_pending = !rx_buf.empty();
            }
            if ((IER & COM1_REG_IER_DATA_AVL_MASK) && rx_pending) {
                data_ptr[0] = 0b0000'0100;  // Received Data Available (010)
            } else if (IER & 0b0000'0010) {
                data_ptr[0] = 0b0000'0010;  // THR Empty (001)
            } else {
                data_ptr[0] = COM1_REG_IIR_NO_INT_PEND;
            }
            if (FCR & COM1_REG_FCR_ENABLE)
                data_ptr[0] |= COM1_REG_IIR_FIFO_16550A;
            break;
        }

        case PIO_PORT_COM1_LCR:
            data_ptr[0] = LCR;
            break;

        case PIO_PORT_COM1_MCR:
            data_ptr[0] = MCR;
            break;

        case PIO_PORT_COM1_LSR: {
            uint8_t v = COM1_REG_LSR_TEMT | COM1_REG_LSR_THRE;
            {
                std::lock_guard<std::mutex> lk(rx_mutex);
                if (!rx_buf.empty())
                    v |= 0b0000'0001;  // Data Ready
            }
            data_ptr[0] = v;
            break;
        }

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
                // serial line.
                if (MCR & COM1_REG_MCR_LOOP) {
                    std::lock_guard<std::mutex> lk(rx_mutex);
                    rx_buf.push_back(static_cast<uint8_t>(data_ptr[0]));
                    break;
                }
                r = write(STDOUT_FILENO, data_ptr, 1);
                if (r == -1) {
                    perror(("COM1::" + std::string(__func__) +
                                ": write").c_str());
                    return -errno;
                }
                // THRE is always reported as 1, so as long as the TX-empty
                // interrupt is enabled we must keep notifying the guest so it
                // drains its xmit buffer past the first FIFO load.
                if (IER & 0b0000'0010) {
                    if ((r = vm->flapIRQLine(COM1_IRQ)))
                        return r;
                }
            } else {               // DLL
                DLL = data_ptr[0];
            }
            break;

        case PIO_PORT_COM1_IER_DLH:
            if (!is_dlab_set()) {  // IER
                IER = data_ptr[0] & COM1_REG_IER_WRITABLE_MASK;
                // Kickstart whichever interrupt source the guest just enabled.
                bool rx_pending;
                {
                    std::lock_guard<std::mutex> lk(rx_mutex);
                    rx_pending = !rx_buf.empty();
                }
                bool should_raise =
                    (IER & 0b0000'0010) ||
                    ((IER & COM1_REG_IER_DATA_AVL_MASK) && rx_pending);
                if (should_raise) {
                    if ((r = vm->flapIRQLine(COM1_IRQ)))
                        return r;
                }
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

void COM1::readerLoop() {
    while (!shutting_down.load(std::memory_order_relaxed)) {
        uint8_t c;
        ssize_t n = ::read(STDIN_FILENO, &c, 1);
        if (n == 1) {
            {
                std::lock_guard<std::mutex> lk(rx_mutex);
                rx_buf.push_back(c);
            }
            if (IER & COM1_REG_IER_DATA_AVL_MASK) {
                vm->flapIRQLine(COM1_IRQ);
            }
        } else if (n == 0) {
            // EOF on stdin (e.g. host pipe closed) — give up.
            return;
        }
        // n < 0: errno set; loop and retry (e.g. EINTR).
    }
}

static struct termios* g_termios_to_restore = nullptr;
static void restore_termios_atexit() {
    if (g_termios_to_restore != nullptr) {
        tcsetattr(STDIN_FILENO, TCSANOW, g_termios_to_restore);
    }
}

COM1::COM1(VM* vm) : IODev(PIO_PORT_COM1_START, PIO_PORT_COM1_SIZE, vm) {
    if (isatty(STDIN_FILENO)) {
        if (tcgetattr(STDIN_FILENO, &saved_termios) == 0) {
            termios_saved = true;
            g_termios_to_restore = &saved_termios;
            std::atexit(restore_termios_atexit);

            struct termios raw = saved_termios;
            cfmakeraw(&raw);
            // Keep output post-processing so dmesg \n still expands to \r\n.
            raw.c_oflag |= OPOST | ONLCR;
            // One-byte reads, no inter-character timeout.
            raw.c_cc[VMIN] = 1;
            raw.c_cc[VTIME] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
        }
    }
    reader_thread = std::thread(&COM1::readerLoop, this);
}

COM1::~COM1() {
    shutting_down.store(true, std::memory_order_relaxed);
    // The reader thread is blocked in read(); detach so process teardown can
    // proceed without an explicit wakeup mechanism.
    if (reader_thread.joinable())
        reader_thread.detach();
    if (termios_saved)
        tcsetattr(STDIN_FILENO, TCSANOW, &saved_termios);
}
