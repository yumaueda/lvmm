#include <fstream>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <kvm.hpp>
#include <vm.hpp>
#include <vcpu.hpp>


// BZIMAGE_PATH = "bzImage";
// INITRAMFS_PATH = "initramfs";


int main() {
    // SuperMigrator Class is needed
    // - SuperMigrator.cfg
    //      - disk_path
    //      - initramfs_path
    //      , etc.
    // - SuperMigrator.Init()
    // - SuperMigrator.Setup()?
    // - SuperMigrator.Boot()

    // init start?
    int  r;
    KVM* kvm;
    VM*  vm;
    // We don't implement membank yet. So there's a limitation of ram_size!
    vm_config vm_conf {
        .vcpu_num = 2,
        .ram_size = static_cast<uint64_t>(1) << 30,
        .kernel_path = "",
        .initramfs_path = "",
    };

    r = KVM::getKVMFD();
    if (r < 0) {
        std::cerr << "KVM::getKVMFD() failed" << std::endl;
        goto out;
    }
    std::cout << "KVM.fd: " << r << std::endl;

    kvm = new KVM(r);


    std::cout << "vm_conf.vcpu_num: " << vm_conf.vcpu_num << std::endl;

    r = kvm->kvmCreateVM(&vm, vm_conf);
    if (r < 0) {
        std::cerr << "kvm->kvmCreateVM() failed" << std::endl;
        goto out_kvm;
    }

    r = vm->initMachine();
    if (r < 0) {
        std::cerr << "vm->initMachine() failed" << std::endl;
        goto out_vm;
    }

    r = vm->initRAM();
    if (r < 0) {
        std::cerr << "vm->initBoot() failed" << std::endl;
        goto out_vm;
    }


    // init end?

    // load linux/initramfs here (pvh?)


    delete vm;
    delete kvm;

    return 0;

out_vm:
    delete vm;

out_kvm:
    delete kvm;

out:
    return -1;
}
