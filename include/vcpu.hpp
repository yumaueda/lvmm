#ifndef VCPU_HPP
#define VCPU_HPP


#include <linux/kvm.h>

#include <baseclass.hpp>
#include <kvm.hpp>


class KVM;


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
        explicit Vcpu(int vcpu_fd, KVM& kvm, int cpu_id);
        ~Vcpu();

        vcpu_regs regs;
        vcpu_sregs sregs;

    private:
        KVM& kvm;
        const int cpu_id;
        kvm_run* run;
};


#endif  // VCPU_HPP
