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
#include <ios>
#include <iostream>

#include <kvm.hpp>
#include <pio.hpp>


int Vcpu::GetRegs(vcpu_regs *regs) {
    int r;
    if ((r = kvmIoctl(KVM_GET_REGS, regs))) {
        std::cerr << "KVM_GET_REGS for vcpu_fd "
            << fd << std::endl;
        return r;
    }
    return 0;
}

int Vcpu::GetSregs(vcpu_sregs *sregs) {
    int r;
    if ((r = kvmIoctl(KVM_GET_SREGS, sregs))) {
        std::cerr << "KVM_GET_SREGS for vcpu_fd "
            << fd << std::endl;
        return r;
    }
    return 0;
}

int Vcpu::SetRegs(vcpu_regs *regs) {
    int r;
    if ((r = kvmIoctl(KVM_SET_REGS, regs))) {
        std::cerr << "KVM_SET_REGS for vcpu_fd "
            << fd << std::endl;
        return r;
    }
    return 0;
}

int Vcpu::SetSregs(vcpu_sregs *sregs) {
    int r;
    if ((r = kvmIoctl(KVM_SET_SREGS, sregs))) {
        std::cerr << "KVM_SET_SREGS for vcpu_fd "
            << fd << std::endl;
        return r;
    }
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

int Vcpu::DumpRegs() {
    int r;
    vcpu_regs regs;

    if ((r = GetRegs(&regs)))
        return r;

    std::cout.setf(std::ios::hex, std::ios::basefield);

    std::cout << "vCPU " << cpu_id << ": vcpu_regs\n";
    // rax, rbx, rcx, rdx
    std::cout
        << "RAX: 0x"    << regs.rax << "\n"
        << "RBX: 0x"    << regs.rbx << "\n"
        << "RCX: 0x"    << regs.rcx << "\n"
        << "RDX: 0x"    << regs.rdx << "\n"
    // rsi, rdi, rsp, rbp
        << "RSI: 0x"    << regs.rsi << "\n"
        << "RDI: 0x"    << regs.rdi << "\n"
        << "RSP: 0x"    << regs.rsp << "\n"
        << "RBP: 0x"    << regs.rbp << "\n"
    // r8, r9, r10, r11
        << "R8: 0x"     << regs.r8  << "\n"
        << "R9: 0x"     << regs.r9  << "\n"
        << "R10: 0x"    << regs.r10 << "\n"
        << "R11: 0x"    << regs.r11 << "\n"
    // r12, r13, r14, r15
        << "R12: 0x"    << regs.r12 << "\n"
        << "R13: 0x"    << regs.r13 << "\n"
        << "R14: 0x"    << regs.r14 << "\n"
        << "R15: 0x"    << regs.r15 << "\n"
    // rip, rflags
        << "RIP: 0x"    << regs.rip << "\n"
        << "RFLAGS: 0x" << regs.rflags << "\n";

    // rflags
    std::cout << "RFLAGS.CF:   " << (regs.rflags & RF_CF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.INIT: " << (regs.rflags & RF_INIT ? "1\n" : "0\n");
    std::cout << "RFLAGS.PF:   " << (regs.rflags & RF_PF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.AF:   " << (regs.rflags & RF_AF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.ZF:   " << (regs.rflags & RF_ZF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.SF:   " << (regs.rflags & RF_SF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.TF:   " << (regs.rflags & RF_TF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.IF:   " << (regs.rflags & RF_IF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.DF:   " << (regs.rflags & RF_DF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.OF:   " << (regs.rflags & RF_OF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.IOPL: " << (regs.rflags & RF_IOPL ? "1\n" : "0\n");
    std::cout << "RFLAGS.NT:   " << (regs.rflags & RF_NT   ? "1\n" : "0\n");
    std::cout << "RFLAGS.MD:   " << (regs.rflags & RF_MD   ? "1\n" : "0\n");
    std::cout << "RFLAGS.RF:   " << (regs.rflags & RF_RF   ? "1\n" : "0\n");
    std::cout << "RFLAGS.VM:   " << (regs.rflags & RF_VM   ? "1\n" : "0\n");
    std::cout << "RFLAGS.AC:   " << (regs.rflags & RF_AC   ? "1\n" : "0\n");
    std::cout << "RFLAGS.VIF:  " << (regs.rflags & RF_VIF  ? "1\n" : "0\n");
    std::cout << "RFLAGS.VIP:  " << (regs.rflags & RF_VIP  ? "1\n" : "0\n");
    std::cout << "RFLAGS.ID:   " << (regs.rflags & RF_ID   ? "1\n" : "0\n");
    std::cout << "RFLAGS.AES:  " << (regs.rflags & RF_AES  ? "1\n" : "0\n");
    std::cout << "RFLAGS.AI:   " << (regs.rflags & RF_AI   ? "1\n" : "0\n");

    std::cout << std::flush;

    std::cout.setf(std::ios::dec, std::ios::basefield);

    return 0;
}

int Vcpu::DumpSregs() {
    int r;
    vcpu_sregs sregs;

    if ((r = GetSregs(&sregs)))
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

int Vcpu::RunOnce() {
    if (Run())
        return 1;

    switch (run->exit_reason) {
        case KVM_EXIT_UNKNOWN:
        case KVM_EXIT_INTR:
            return 0;

        case KVM_EXIT_DEBUG:
        case KVM_EXIT_HLT:
            return 1;

        case KVM_EXIT_IO:
            for (uint32_t i = 0; i < run->io.count; ++i) {
                if(vm->pio_handler[run->io.port][run->io.direction](
                        reinterpret_cast<char*>(run)+run->io.data_offset,
                        run->io.size)) {
                    return 1;
                }
            }
            return 0;

        case KVM_EXIT_MMIO:
            return 1;  // unimplemented

        default:
        /*
         *  no plan for support these below
         *
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
            return 1;
    }
}

int Vcpu::RunLoop() {
    int r;

    std::cout << "Vcpu::" << __func__ << ": cpu " << cpu_id
        << " is running" << std::endl;

    while (true) {
        if(RunOnce()) {
            std::cerr << "Vcpu::" << __func__ << ": cpu " << cpu_id
                << ": can not keep vCPU running" << std::endl;
            std::cerr << "exit_reason: " << run->exit_reason << std::endl;

            switch (run->exit_reason) {
                case KVM_EXIT_FAIL_ENTRY:
                    std::cerr << "hardware_entry_failure_reason: "
                        << run->fail_entry.hardware_entry_failure_reason
                        << std::endl;

                // default...
            }

            if ((r = DumpRegs()))
                return r;
            if ((r = DumpSregs()))
                return r;

            return 1;
        }
    }
}

Vcpu::Vcpu(int vcpu_fd, KVM* kvm, VM* vm, int cpu_id)
    : BaseClass(vcpu_fd), kvm(kvm), vm(vm), cpu_id(cpu_id) {
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
