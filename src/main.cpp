#include <iostream>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <kvm.hpp>
#include <vm.hpp>
#include <vcpu.hpp>

int main(void) {
    // SuperMigrator Class is needed
    // - SuperMigrator.cfg
    //      - disk_path
    //      - initrd_path
    //      , etc.
    // - SuperMigrator.Init()
    // - SuperMigrator.Setup()?
    // - SuperMigrator.Boot()

    int r;
    if ((r = open(DEV_KVM, O_RDWR)) < 0)
        std::cout<< (std::string(__func__) + ": Could not open " + DEV_KVM + ".") << std::endl;
    std::cout << "KVM.fd: " << r << std::endl;

    KVM *kvm = new KVM(r);
    VM *vm;

    // We don't implement membank yet. So there's a limitation of ram_size!
    const uint64_t ram_size = static_cast<uint64_t>(1) << 30;
    const int vcpu_num = 2;

    std::cout << "main() ram_size: " << ram_size << std::endl;
    r = kvm->kvmCreateVM(&vm, ram_size, vcpu_num);
    if (r < 0) {
        std::cerr << "kvm->kvmCreateVM() failed" << std::endl;
        goto out_tmp;
    }


    delete vm;
    delete kvm;
    return 0;

    delete vm;
out_tmp:
    delete kvm;
    return -1;
}
