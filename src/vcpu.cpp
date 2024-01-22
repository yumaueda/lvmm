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
#include <iomanip>
#include <ios>
#include <iostream>
#include <string>
#include <vector>

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
    int r;
    vcpu_sregs sregs;
    segment_descriptor sd, others;

    if ((r = GetSregs(&sregs)))
        return r;

    if (!is_64bit_boot) {
        sregs.cs.base  = 0;
        sregs.cs.limit = 0xffff'ffff;
        sregs.cs.g     = 1;
        sregs.ds.base  = 0;
        sregs.ds.limit = 0xffff'ffff;
        sregs.ds.g     = 1;
        sregs.fs.base  = 0;
        sregs.fs.limit = 0xffff'ffff;
        sregs.fs.g     = 1;
        sregs.gs.base  = 0;
        sregs.gs.limit = 0xffff'ffff;
        sregs.gs.g     = 1;
        sregs.es.base  = 0;
        sregs.es.limit = 0xffff'ffff;
        sregs.es.g     = 1;
        sregs.ss.base  = 0;
        sregs.ss.limit = 0xffff'ffff;
        sregs.ss.g     = 1;

        sregs.cs.db = sregs.ss.db = 1;
        sregs.cr0 |= CR0_PE;

        if ((r = SetSregs(&sregs)))
            return r;

        return 0;
    }

    sd = {
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

    others = sd;
    others.selector = 2 << SEG_DESC_SELECTOR_IDX_SHIFT
        | SEG_DESC_SELECTOR_TI_GDT
        | SEG_DESC_SELECTOR_RPL_KERNEL;
    others.type = SEG_DESC_TYPE_DATA
        | SEG_DESC_TYPE_DATA_A
        | SEG_DESC_TYPE_DATA_W;

    // reconsider order
    //
    // CR0.PG=1,  CR0.PE=1 ------------------------> Paging Enabled
    // CR4.PAE=1, MSR.IA32_EFER.LME=1, CR4.LA57=0 -> 4-Level Paging
    sregs.cr3  = BOOT_PAGETABLE_BASE;
    sregs.cr4  = CR4_PAE;
    sregs.cr0  = CR0_PE | CR0_MP | CR0_ET | CR0_NE | CR0_WP | CR0_AM | CR0_PG;
    sregs.efer = MSR_IA32_EFER_LME | MSR_IA32_EFER_LMA;
    sregs.cs   = sd;
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
        << std::setfill('0')
        << "RAX: 0x"    << std::setw(16) << regs.rax << " "
        << "RBX: 0x"    << std::setw(16) << regs.rbx << " "
        << "RCX: 0x"    << std::setw(16) << regs.rcx << " "
        << "RDX: 0x"    << std::setw(16) << regs.rdx << "\n"
    // rsi, rdi, rsp, rbp
        << "RSI: 0x"    << std::setw(16) << regs.rsi << " "
        << "RDI: 0x"    << std::setw(16) << regs.rdi << " "
        << "RSP: 0x"    << std::setw(16) << regs.rsp << " "
        << "RBP: 0x"    << std::setw(16) << regs.rbp << "\n"
    // r8, r9, r10, r11
        << "R8:  0x"    << std::setw(16) << regs.r8  << " "
        << "R9:  0x"    << std::setw(16) << regs.r9  << " "
        << "R10: 0x"    << std::setw(16) << regs.r10 << " "
        << "R11: 0x"    << std::setw(16) << regs.r11 << "\n"
    // r12, r13, r14, r15
        << "R12: 0x"    << std::setw(16) << regs.r12 << " "
        << "R13: 0x"    << std::setw(16) << regs.r13 << " "
        << "R14: 0x"    << std::setw(16) << regs.r14 << " "
        << "R15: 0x"    << std::setw(16) << regs.r15 << "\n"
    // rip, rflags
        << "RIP: 0x"    << std::setw(16) << regs.rip << "\n"
        << "RFLAGS: 0x" << std::setw(16) << regs.rflags << "\n";

    // rflags
    std::cout
        << "CF:   " << (regs.rflags & RF_CF   ? "1 " : "0 ")
        << "INIT: " << (regs.rflags & RF_INIT ? "1 " : "0 ")
        << "PF:   " << (regs.rflags & RF_PF   ? "1 " : "0 ")
        << "AF:   " << (regs.rflags & RF_AF   ? "1 " : "0 ")
        << "ZF:   " << (regs.rflags & RF_ZF   ? "1 " : "0 ")
        << "SF:   " << (regs.rflags & RF_SF   ? "1 " : "0 ")
        << "TF:   " << (regs.rflags & RF_TF   ? "1\n" : "0\n")
        << "IF:   " << (regs.rflags & RF_IF   ? "1 " : "0 ")
        << "DF:   " << (regs.rflags & RF_DF   ? "1 " : "0 ")
        << "OF:   " << (regs.rflags & RF_OF   ? "1 " : "0 ")
        << "IOPL: " << (regs.rflags & RF_IOPL ? "1 " : "0 ")
        << "NT:   " << (regs.rflags & RF_NT   ? "1 " : "0 ")
        << "MD:   " << (regs.rflags & RF_MD   ? "1 " : "0 ")
        << "RF:   " << (regs.rflags & RF_RF   ? "1\n" : "0\n")
        << "VM:   " << (regs.rflags & RF_VM   ? "1 " : "0 ")
        << "AC:   " << (regs.rflags & RF_AC   ? "1 " : "0 ")
        << "VIF:  " << (regs.rflags & RF_VIF  ? "1 " : "0 ")
        << "VIP:  " << (regs.rflags & RF_VIP  ? "1 " : "0 ")
        << "ID:   " << (regs.rflags & RF_ID   ? "1 " : "0 ")
        << "AES:  " << (regs.rflags & RF_AES  ? "1 " : "0 ")
        << "AI:   " << (regs.rflags & RF_AI   ? "1\n" : "0\n");

    std::cout << std::flush;

    std::cout.setf(std::ios::dec, std::ios::basefield);

    return 0;
}

void Vcpu::DumpSegmentDescriptor(segment_descriptor& sd) {
    std::cout.setf(std::ios::hex, std::ios::basefield);

    std::cout << std::setfill('0')
        << "base:     0x" << std::setw(16) << sd.base     << "\n"
        << "limit:    0x" << std::setw(8)  << sd.limit    << "\n"
        << "selector: 0x" << std::setw(4)  << sd.selector << "\n"
        << "type:     0x" << std::setw(2)  << sd.type     << "\n";

    std::cout
        << "present:  " << (sd.present ? "1 " : "0 ")
        << "dpl:      " << (sd.dpl     ? "1 " : "0 ")
        << "db:       " << (sd.db      ? "1 " : "0 ")
        << "s:        " << (sd.s       ? "1\n" : "0\n")
        << "l:        " << (sd.l       ? "1 " : "0 ")
        << "g:        " << (sd.g       ? "1 " : "0 ")
        << "avl:      " << (sd.avl     ? "1\n" : "0\n");

    std::cout << std::flush;

    std::cout.setf(std::ios::dec, std::ios::basefield);
}

void Vcpu::DumpDescriptorTable(descriptor_table& dt) {
    std::cout.setf(std::ios::hex, std::ios::basefield);

    std::cout << std::setfill('0')
        << "base:  0x" << std::setw(16) << dt.base  << "\n"
        << "limit: 0x" << std::setw(4)  << dt.limit << std::endl;

    std::cout.setf(std::ios::dec, std::ios::basefield);
}

int Vcpu::DumpSregs() {
    int r;
    vcpu_sregs sregs;

    if ((r = GetSregs(&sregs)))
        return r;

    std::cout.setf(std::ios::hex, std::ios::basefield);

    std::cout << "vCPU " << cpu_id << ": vcpu_sregs\n";

    // cs, ds, es, fs, gs, ss, tr, ldt
    std::vector<std::pair<std::string, SegmentDescriptorPointer>> sdmap = {
        {"CS",  &vcpu_sregs::cs},
        {"DS",  &vcpu_sregs::ds},
        {"ES",  &vcpu_sregs::es},
        {"FS",  &vcpu_sregs::fs},
        {"GS",  &vcpu_sregs::gs},
        {"SS",  &vcpu_sregs::ss},
        {"TR",  &vcpu_sregs::tr},
        {"LDR", &vcpu_sregs::ldt},
    };

    for (auto& pair : sdmap) {
        std::cout << pair.first << std::endl;
        DumpSegmentDescriptor(sregs.*pair.second);
    }

    // gdt, idt
    std::vector<std::pair<std::string, DescriptorTablePointer>> dtmap = {
        {"GDT", &vcpu_sregs::gdt},
        {"IDT", &vcpu_sregs::idt},
    };

    for (auto& pair : dtmap) {
        std::cout << pair.first << std::endl;
        DumpDescriptorTable(sregs.*pair.second);
    }

    // cr0, cr2, cr3, cr4, cr8
    std::cout << "CR0: 0x" << std::setfill('0') << std::setw(16)
        << sregs.cr0 << "\n";
    std::cout
        << "PE: " << (sregs.cr0 & CR0_PE ? "1 " : "0 ")
        << "MP: " << (sregs.cr0 & CR0_MP ? "1 " : "0 ")
        << "EM: " << (sregs.cr0 & CR0_EM ? "1 " : "0 ")
        << "TS: " << (sregs.cr0 & CR0_TS ? "1 " : "0 ")
        << "ET: " << (sregs.cr0 & CR0_ET ? "1 " : "0 ")
        << "NE: " << (sregs.cr0 & CR0_NE ? "1\n" : "0\n")
        << "WP: " << (sregs.cr0 & CR0_WP ? "1 " : "0 ")
        << "AM: " << (sregs.cr0 & CR0_AM ? "1 " : "0 ")
        << "NW: " << (sregs.cr0 & CR0_NW ? "1 " : "0 ")
        << "CD: " << (sregs.cr0 & CR0_CD ? "1 " : "0 ")
        << "PG: " << (sregs.cr0 & CR0_PG ? "1\n" : "0\n");

    std::cout << std::setfill('0')
        << "CR2: 0x" << std::setw(16) << sregs.cr2 << "\n"
        << "CR3: 0x" << std::setw(16) << sregs.cr3 << "\n";
    std::cout
        << "PWT: "    << (sregs.cr3 & CR3_PWT  ? "1 " : "0 ")
        << "PCD: "    << (sregs.cr3 & CR3_PCD  ? "1 " : "0 ")
        << "PDBR: 0x" << (sregs.cr3 & CR3_PDBR_MASK) << "\n";

    std::cout << "CR4: 0x" << std::setfill('0') << std::setw(16)
        << sregs.cr4 << "\n";
    std::cout
        << "CR4.VME:        " << (sregs.cr4 & CR4_VME ? "1 " : "0 ")
        << "CR4.PVI:        " << (sregs.cr4 & CR4_PVI ? "1 " : "0 ")
        << "CR4.TSD:        " << (sregs.cr4 & CR4_TSD ? "1 " : "0 ")
        << "CR4.DE:         " << (sregs.cr4 & CR4_DE ? "1 " : "0 ")
        << "CR4.PSE:        " << (sregs.cr4 & CR4_PSE ? "1 " : "0 ")
        << "CR4.PAE:        " << (sregs.cr4 & CR4_PAE ? "1\n" : "0\n")
        << "CR4.MCE:        " << (sregs.cr4 & CR4_MCE ? "1 " : "0 ")
        << "CR4.PGE:        " << (sregs.cr4 & CR4_PGE ? "1 " : "0 ")
        << "CR4.PCE:        " << (sregs.cr4 & CR4_PCE ? "1 " : "0 ")
        << "CR4.OSFXSR:     " << (sregs.cr4 & CR4_OSFXSR ? "1 " : "0 ")
        << "CR4.OSXMMEXCPT: " << (sregs.cr4 & CR4_OSXMMEXCPT ? "1 " : "0 ")
        << "CR4.UMIP:       " << (sregs.cr4 & CR4_UMIP ? "1\n" : "0\n")
        << "CR4.VMXE:       " << (sregs.cr4 & CR4_VMXE ? "1 " : "0 ")
        << "CR4.SMXE:       " << (sregs.cr4 & CR4_SMXE ? "1 " : "0 ")
        << "CR4.FSGSBASE:   " << (sregs.cr4 & CR4_FSGSBASE ? "1 " : "0 ")
        << "CR4.PCIDE:      " << (sregs.cr4 & CR4_PCIDE ? "1 " : "0 ")
        << "CR4.OSXSAVE:    " << (sregs.cr4 & CR4_OSXSAVE ? "1 " : "0 ")
        << "CR4.SMEP:       " << (sregs.cr4 & CR4_SMEP ? "1\n" : "0\n")
        << "CR4.SMAP:       " << (sregs.cr4 & CR4_SMAP ? "1 " : "0 ")
        << "CR4.PKE:        " << (sregs.cr4 & CR4_PKE ? "1 " : "0 ")
        << "CR4.CET:        " << (sregs.cr4 & CR4_CET ? "1 " : "0 ")
        << "CR4.PKS:        " << (sregs.cr4 & CR4_PKS ? "1\n" : "0\n");

    std::cout << "CR8: 0x" << std::setfill('0') << std::setw(16)
        << sregs.cr8 << "\n";

    // efer
    std::cout << "EFER: 0x" << std::setfill('0') << std::setw(16)
        << sregs.efer << "\n";
    std::cout
        << "EFER.SCE:   " << (sregs.efer & MSR_IA32_EFER_SCE ? "1 " : "0 ")
        << "EFER.LME:   " << (sregs.efer & MSR_IA32_EFER_LME ? "1 " : "0 ")
        << "EFER.LMA:   " << (sregs.efer & MSR_IA32_EFER_LMA ? "1 " : "0 ")
        << "EFER.NXE:   " << (sregs.efer & MSR_IA32_EFER_NXE ? "1\n" : "0\n")
        << "EFER.SVME:  " << (sregs.efer & MSR_IA32_EFER_SVME ? "1 " : "0 ")
        << "EFER.LMSLE: " << (sregs.efer & MSR_IA32_EFER_LMSLE ? "1 " : "0 ")
        << "EFER.FFXSR: " << (sregs.efer & MSR_IA32_EFER_FFXSR ? "1 " : "0 ")
        << "EFER.TCE:   " << (sregs.efer & MSR_IA32_EFER_TCE ? "1\n" : "0\n");

    // apic_base
    std::cout << "APIC_BASE: 0x" << std::setfill('0') << std::setw(16)
        << sregs.apic_base << "\n";

    // TODO: interrupt_bitmap

    std::cout << std::flush;

    std::cout.setf(std::ios::dec, std::ios::basefield);

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
                if (vm->pio_handler[run->io.port][run->io.direction](
                        run->io.port,
                        reinterpret_cast<char*>(run)+run->io.data_offset,
                        run->io.size)
                ) {
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
    uint64_t ic = 0;  // tmp

    std::cout << "Vcpu::" << __func__ << ": cpu " << cpu_id
        << " is running" << std::endl;

    while (true) {
        ic++;
        if (RunOnce()) {
            std::cerr << "Vcpu::" << __func__ << ": cpu " << cpu_id
                << ": ic: " << ic << " can not keep vCPU running" << std::endl;
            std::cerr << "exit_reason: " << run->exit_reason << std::endl;

            switch (run->exit_reason) {
                case KVM_EXIT_FAIL_ENTRY:
                    std::cerr << "hardware_entry_failure_reason: "
                        << run->fail_entry.hardware_entry_failure_reason
                        << std::endl;

                // default...
            }

            // FIXME: calling dump funcs here can cause simultaneous output
            // from multiple threads
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
