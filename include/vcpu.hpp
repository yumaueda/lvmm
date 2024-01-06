#ifndef VCPU_HPP
#define VCPU_HPP


#include <linux/kvm.h>

#include <baseclass.hpp>
#include <kvm.hpp>


class KVM;

constexpr uint64_t CR0_PE    = 1;
constexpr uint64_t CR0_MP    = 1 << 1;
constexpr uint64_t CR0_EM    = 1 << 2;
constexpr uint64_t CR0_TS    = 1 << 3;
constexpr uint64_t CR0_ET    = 1 << 4;
constexpr uint64_t CR0_NE    = 1 << 5;
constexpr uint64_t CR0_WP    = 1 << 16;
constexpr uint64_t CR0_AM    = 1 << 18;
constexpr uint64_t CR0_NW    = 1 << 29;
constexpr uint64_t CR0_CD    = 1 << 30;
constexpr uint64_t CR0_PG    = 1 << 31;

constexpr uint64_t CR4_PAE   = 1 << 5;

constexpr uint64_t RF_INIT   = 1 << 2;

struct vcpu_regs {
    uint64_t rax, rbx, rcd, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rflags, rip;
};

struct descriptor_table {
    uint64_t base;
    uint16_t limit;
    uint16_t padding[3];
};

struct segment_descriptor {
    uint64_t base;
    uint32_t limit;
    uint16_t selector;
    uint8_t  type;
    uint8_t  present, dpl, db, s, l, g, avl;
    uint8_t  unsusable;
    uint8_t  padding;
};

struct vcpu_sregs {
    segment_descriptor cs, ds, es, fs, gs, ss;
    segment_descriptor tr, ldt;
    descriptor_table gdt, idt;
    uint64_t cr0, cr2, cr3, cr4, cr8;
    uint64_t efer;
    uint64_t apic_base;
    uint64_t interrupt_bitmap[(KVM_NR_INTERRUPTS+63)/64];
};

struct vcpu_dregs {
    uint64_t db[4];
    uint64_t dr6;
    uint64_t dr7;
    uint64_t flags;
    uint64_t reserved[9];
};

class Vcpu : public BaseClass {
    public:
        explicit Vcpu(int vcpu_fd, KVM& kvm, int cpu_id);
        ~Vcpu();

        int InitRegs(uint64_t rip, uint64_t rsi);
        int InitSregs(bool is_elfclass64);
        int GetRegs(vcpu_regs *regs);
        int GetSregs(vcpu_sregs *sregs);
        int SetRegs(vcpu_regs *regs);
        int SetSregs(vcpu_sregs *sregs);

    private:
        KVM& kvm;
        const int cpu_id;

        kvm_run* run = static_cast<kvm_run*>(nullptr);

        vcpu_regs  regs;
        vcpu_sregs sregs;
        vcpu_dregs dregs;
};


#endif  // VCPU_HPP
