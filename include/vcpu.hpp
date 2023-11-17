#ifndef VCPU_HPP
#define VCPU_HPP


#include <vm.hpp>

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
        explicit Vcpu(VM& vm, int cpu_id, int vcpu_fd) : vm(vm), cpu_id(cpu_id), vcpu_fd(vcpu_fd) {
            dummy = new int;
        }

        ~Vcpu() {
            close(this->vcpu_fd);
            std::cout << "closed vcpu_fd: " << this->vcpu_fd << std::endl;
            delete dummy;
        }

        int *dummy;
        struct vcpu_regs regs;
        struct vcpu_sregs sresg;

    private:
        VM& vm;
        const int cpu_id;
        const int vcpu_fd;

};


#endif  // VCPU_HPP
