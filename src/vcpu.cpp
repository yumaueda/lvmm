#include <cassert>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/kvm.h>

#include <kvm.hpp>
#include <vcpu.hpp>


int Vcpu::GetRegs(vcpu_regs *regs) {
    if (kvmIoctl(KVM_GET_REGS, regs))
        std::cerr << "KVM_GET_REGS for vcpu_fd "
            << fd << std::endl;
    return 0;
}

int Vcpu::GetSregs(vcpu_sregs *sregs) {
    if (kvmIoctl(KVM_GET_SREGS, sregs))
        std::cerr << "KVM_GET_SREGS for vcpu_fd "
            << fd << std::endl;
    return 0;
}

int Vcpu::SetRegs(vcpu_regs *regs) {
    if (kvmIoctl(KVM_SET_REGS, regs))
        std::cerr << "KVM_SET_REGS for vcpu_fd "
            << fd << std::endl;
    return 0;
}

int Vcpu::SetSregs(vcpu_sregs *sregs) {
    if (kvmIoctl(KVM_SET_SREGS, sregs))
        std::cerr << "KVM_SET_SREGS for vcpu_fd "
            << fd << std::endl;
    return 0;
}

int Vcpu::InitRegs(uint64_t rip, uint64_t rsi) {
    int r;
    vcpu_regs  regs;

    if ((r = GetRegs(&regs)))
        return r;

    regs.rip = rip;
    regs.rsi = rsi;
    regs.rflags = RF_INIT;

    if ((r = SetRegs(&regs)))
        return r;

    return 0;
}

int Vcpu::InitSregs(bool is_elfclass64) {
    assert(!is_elfclass64);

    int r;
    vcpu_sregs  sregs;

    if ((r = GetSregs(&sregs)))
        return r;

    //sregs.CR3 =
    sregs.cr4 = CR4_PAE;
    sregs.cr0 = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;

    // MSR.IA32_EFER = 
    // sregs.CS
    // sregs.DS
    // sregs.ES
    // sregs.FS
    // sregs.GS
    // sregs.SS

    if ((r = SetSregs(&sregs)))
        return r;

    std::cout << "Vcpu::" << __func__
        << ": incomplete implementation!!" << std::endl;

    return 0;
}

Vcpu::Vcpu(int vcpu_fd, KVM& kvm, int cpu_id)
    : BaseClass(vcpu_fd), kvm(kvm), cpu_id(cpu_id) {
    std::cout << "Constructing Vcpu..." << std::endl;

    run = static_cast<struct kvm_run*>(mmap(NULL, kvm.mmap_size
                , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (run == MAP_FAILED) {
        perror("Vcpu.run: mmap");
        // The type of exception should be detailed.
        throw std::runtime_error("Vcpu::" + std::string(__func__)
                + ": " + strerror(errno));
    } else {
        std::cout << "Vcpu.run mmaped: " << run << std::endl;
    }

    std::cout << "Constructed Vcpu." << std::endl;
}

Vcpu::~Vcpu() {}
