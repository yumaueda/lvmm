#ifndef VCPU_HPP
#define VCPU_HPP


#include <linux/kvm.h>

#include <baseclass.hpp>


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

class Vcpu : public BaseClass {
    public:
        explicit Vcpu(int vcpu_fd, int cpu_id, int mmap_size);
        ~Vcpu();

        struct vcpu_regs regs;
        struct vcpu_sregs sresg;

    private:
        const int cpu_id;
        const int mmap_size;
        struct kvm_run* run;
};


#endif  // VCPU_HPP
