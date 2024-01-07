#include <cassert>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/kvm.h>
#include <kvm.hpp>
#include <paging.hpp>
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

int Vcpu::InitSregs(bool is_64bit) {
    assert(!is_64bit);

    int r;
    vcpu_sregs sregs;
    segment_descriptor cs = {
        .base     = 0,
        .limit    = 0xffff'ffff,
        .selector = 1 << SEG_DESC_SELECTOR_IDX_SHIFT
                  | SEG_DESC_SELECTOR_TI_GDT
                  | SEG_DESC_SELECTOR_RPL_KERNEL,
        .type     = SEG_DESC_TYPE_CODE
                  | SEG_DESC_TYPE_CODE_A
                  | SEG_DESC_TYPE_CODE_R,
        .present  = 1,
        .dpl      = SEG_DESC_DPL_KERNEL,
        .db       = SEG_DESC_DB_EX_LSET,    // .db must be cleared when .l=1
        .s        = SEG_DESC_TYPE_FLAG_CD,
        .l        = SEG_DESC_L_64BIT_MODE,
        .g        = SEG_DESC_GRAN_4KB,
        .avl      = 0,
    };

    segment_descriptor others = cs;
    others.selector = 2 << SEG_DESC_SELECTOR_IDX_SHIFT
        | SEG_DESC_SELECTOR_TI_GDT
        | SEG_DESC_SELECTOR_RPL_KERNEL;
    others.type = SEG_DESC_TYPE_DATA
        | SEG_DESC_TYPE_DATA_A
        | SEG_DESC_TYPE_DATA_W;

    if ((r = GetSregs(&sregs)))
        return r;

    // reconsider order
    //
    // CR0.PG=1,  CR0.PE=1 ------------------------> Paging Enabled
    // CR4.PAE=1, MSR.IA32_EFER.LME=1, CR4.LA57=0 -> 4-Level Paging
    sregs.cr3  = BOOT_PAGETABLE_BASE;
    sregs.cr4  = CR4_PAE;
    sregs.cr0  = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
    sregs.efer = MSR_IA32_EFER_LME | MSR_IA32_EFER_LMA;
    sregs.cs   = cs;
    sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = others;

    if ((r = SetSregs(&sregs)))
        return r;

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
