/*
 *  src/main.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <cstdint>
#include <iostream>

#include <boot.hpp>
#include <kvm.hpp>
#include <vcpu.hpp>
#include <vm.hpp>


int main() {
    int  r;
    KVM* kvm;
    VM*  vm;
    vm_config vm_conf {
        .vcpu_num = 1,
        .ram_size = static_cast<uint64_t>(1) << 30,
        .kernel_path = "bzImage",
        .initramfs_path = "initramfs",
        .is_64bit_boot = false,
    };

    r = KVM::getKVMFD();
    if (r < 0)
        return -1;

    kvm = new KVM(r);
    r = kvm->kvmCreateVM(&vm, vm_conf);
    if (r < 0) {
        std::cerr << "kvm->kvmCreateVM() failed" << std::endl;
        return -1;
    }

    r = vm->initMachine();
    if (r) {
        std::cerr << "vm->initMachine() failed" << std::endl;
        return -1;
    }

    //r = vm->initRAM("console=ttyS0 earlyprintk=serial noapic noacpi notsc nowatchdog nmi_watchdog=0 debug apic=debug show_lapic=all mitigations=off lapic tsc_early_khz=2000 dyndbg=\"file arch/x86/kernel/smpboot.c +plf ; file drivers/net/virtio_net.c +plf\" pci=realloc=off virtio_pci.force_legacy=1 rdinit=/init init=/init");
    r = vm->initRAM("console=ttyS0 earlyprintk=serial noapic noacpi notsc nowatchdog nmi_watchdog=0 debug apic=debug show_lapic=all mitigations=off lapic tsc_early_khz=2000 rdinit=/init init=/init");
    if (r) {
        std::cerr << "vm->initRAM() failed" << std::endl;
        return -1;
    }

    r = vm->Boot();

    return 0;
}
