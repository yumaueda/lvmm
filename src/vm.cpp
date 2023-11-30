#include <cstdint>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <new>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/kvm.h>
#include <bios.hpp>
#include <bootloader.hpp>
#include <vm.hpp>


int VM::allocGuestRAM() {
    // not caring about hugetlbpage
    ram_start = mmap(NULL, vm_conf.ram_size, (PROT_READ|PROT_WRITE),
                        (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);
    if (ram_start == MAP_FAILED) {
        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
        return -errno;
    }

    if (madvise(ram_start, vm_conf.ram_size, MADV_MERGEABLE) < 0) {
        std::cerr << "madvise failed: " << std::strerror(errno) << std::endl;
        return -errno;
    }

    return 0;
}


int VM::setUserMemRegion() {
    // TMP implementation
    // We don't implement membank yet. So there's a limitation of ram_size!
    // RECOMMENDED: userspace_addr[0:20] == guest_phys_addr[0:20]
    user_memory_region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = vm_conf.ram_size,
        .userspace_addr = reinterpret_cast<uint64_t>(ram_start),
    };

    return kvmIoctl(KVM_SET_USER_MEMORY_REGION, &user_memory_region);
}


int VM::createVcpu() {
    int r;
    vcpus = (Vcpu*)operator new[](vm_conf.vcpu_num*sizeof(Vcpu));

    for (int i = 0; i < vm_conf.vcpu_num; i++) {
        r = kvmIoctl(KVM_CREATE_VCPU, i);
        if (r < 0) {
            std::cerr << "KVM_CREATE_VCPU failed." << std::endl;
            return -errno;
        }
        // error handling
        new(&vcpus[i]) Vcpu(r, kvm, i);
        std::cout << "&vcpus[" << i << "]: " << &vcpus[i] << std::endl;
    }

    return 0;
}


int VM::initMachine() {
    int r;
    for (const InitMachineFunc e : init_machine_func) {
        r = (this->*e)();
        if (r < 0)
            return r;
    }
    return 0;
}


int VM::initBoot() {
    ebda ebda_data = gen_ebda(vm_conf.vcpu_num);
    std::cout << "ebda_data.fps.checksum: "
        << ebda_data.fps.checksum+0 << std::endl;
    std::cout << "ebda_data.ctable.checksum: "
        << ebda_data.ctable.checksum+0 << std::endl;
    return 0;
}


VM::VM(int vm_fd, KVM& kvm, vm_config vm_conf)\
        : BaseClass(vm_fd), kvm(kvm), vm_conf(vm_conf) {
    std::cout << "Constructing VM..." << std::endl;

    // Currently we only assumes the case where unrestricted_guest == 1.
    // So we won't call following APIs:
    // - KVM_SET_TSS
    // - KVM_SET_IDENTITY_MAP

    // Create VIOAPIC, VPIC. Set up future vCPUs to use them as a local APIC.
    // - GSI00-15 -> IOAPIC/PIC
    // - GIS16-23 -> IOAPIC
    kvmIoctlCtor(KVM_CREATE_IRQCHIP);
    kvmIoctlCtor(KVM_CREATE_PIT2, &pit_config);
}


VM::~VM() {
    /*
    errno = 0;
    std::cout << "Destructing VM..." << std::endl;
    for (int i = 0; i < vm_conf.vcpu_num; i++) {
        std::cout << "deleted VM.vcpus[i]" << std::endl;
        delete vcpus[i];
    }
    if (vcpus != NULL) {
        std::cout << "deleted VM.vcpus" << std::endl;
        delete vcpus;
    }
    if (ram_start != NULL) {
        if (munmap(ram_start, vm_conf.ram_size) < 0)
            std::cerr << "munmap VM.ram_start failed: " << std::strerror(errno) << std::endl;
        else
            std::cout << "munmapped VM.ram_start" << std::endl;
    }
    if (fd >= 0) {
        close(fd);
        std::cout << "closed VM.fd" << std::endl;
    }
    */
}
