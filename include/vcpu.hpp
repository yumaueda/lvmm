#ifndef VCPU_HPP
#define VCPU_HPP


#include <kvm.hpp>
#include <vm.hpp>

class KVM;
class VM;

struct vcpu_sregs {
    uint64_t cr0, cr2, cr3, cr4, cr8;
    // and so on...
};

struct vcpu_regs {
    uint64_t rax, rbx, rcd, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t rip;
    // and so on...
};


class Vcpu {
    public:
        explicit Vcpu(VM& vm, int cpu_id, int vcpu_fd);
        ~Vcpu();

        int *dummy;
        struct vcpu_regs regs;
        struct vcpu_sregs sresg;

    private:
        VM& vm;
        const int cpu_id;
        const int vcpu_fd;
};


#endif  // VCPU_HPP
