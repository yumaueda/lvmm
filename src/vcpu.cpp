/*
 *  src/vcpu.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <vcpu.hpp>

#include <sys/mman.h>
#include <unistd.h>
#include <linux/kvm.h>

#include <cassert>
#include <cerrno>
#include <iostream>

#include <kvm.hpp>


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

int Vcpu::InitSregs(bool is_64bit_boot) {
    assert(!is_64bit_boot);  // to be implemented

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

int Vcpu::Run() {
    int r;

    if ((r = kvmIoctl(KVM_RUN, 0))) {
        if (errno != EAGAIN && errno != EINTR) {
            perror("Vcpu::run()");
            return r;
        }
    }

    return r;
}

bool Vcpu::RunOnce() {
    Run();

    switch (run->exit_reason) {
        case KVM_EXIT_UNKNOWN:
        case KVM_EXIT_INTR:
        case KVM_EXIT_DEBUG:
            return true;

        case KVM_EXIT_HLT:
            return false;

        case KVM_EXIT_IO:
            // call the handler corresponding to the error here
            return true;

        case KVM_EXIT_MMIO:
            return true;
        default:
        /*
         *  KVM_EXIT_EXCEPTION
         *  KVM_EXIT_HYPERCALL
         *  KVM_EXIT_IRQ_WINDOW_OPEN
         *  KVM_EXIT_SHUTDOWN
         *  KVM_EXIT_FAIL_ENTRY
         *  KVM_EXIT_SET_TPR
         *  KVM_EXIT_TPR_ACCESS
         *  KVM_EXIT_S390_SIEIC
         *  KVM_EXIT_S390_RESET
         *  KVM_EXIT_DCR
         *  KVM_EXIT_NMI
         *  KVM_EXIT_INTERNAL_ERROR
         *  KVM_EXIT_OSI
         *  KVM_EXIT_PAPR_HCALL
         *  KVM_EXIT_S390_UCONTROL
         *  KVM_EXIT_WATCHDOG
         *  KVM_EXIT_S390_TSCH
         *  KVM_EXIT_EPR
         *  KVM_EXIT_SYSTEM_EVENT
         *  KVM_EXIT_S390_STSI
         *  KVM_EXIT_IOAPIC_EOI
         *  KVM_EXIT_HYPERV
         *
         *  and unexpected values
         */
            return false;
    }
}

Vcpu::Vcpu(int vcpu_fd, KVM* kvm, int cpu_id)
    : BaseClass(vcpu_fd), kvm(kvm), cpu_id(cpu_id) {
    std::cout << "Constructing Vcpu..." << std::endl;

    run = static_cast<struct kvm_run*>(mmap(NULL, kvm->mmap_size
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
