#ifndef VCPU_HPP
#define VCPU_HPP


#include <vm.hpp>

struct vcpu_sregs {
    uint64_t cr0, cr2, cr3, cr4, cr8;
};

struct vcpu_regs {
    uint64_t rax, rbx, rcd, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t rip;
};

class Vcpu {
    public:
        Vcpu() {
        }

        ~Vcpu() {
        }

        struct vcpu_regs regs;
        int cpu_id;

    private:
        VM& vm;
};


#endif  // VCPU_HPP
