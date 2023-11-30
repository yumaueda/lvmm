#include <algorithm>
#include <cstdint>
#include <cstdio>
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
        perror(("VM::" + std::string(__func__) + ": mmap").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__ << ": VM.ram_start mmaped: "
        << ram_start << std::endl;

    if (madvise(ram_start, vm_conf.ram_size, MADV_MERGEABLE) < 0) {
        perror(("VM::" + std::string(__func__) + ": madvise").c_str());
        return -errno;
    }
    std::cout << "VM::" << __func__
        << ": VM.ram_start madvised as MERGEABLE: "
        << ram_start << std::endl;

    return 0;
}


int VM::setUserMemRegion() {
    // TMP implementation
    // We don't implement membank yet. So there's a limitation of ram_size!
    // RECOMMENDED: userspace_addr[0:20] == guest_phys_addr[0:20]
    int r;
    user_memory_region = {
        .slot = 0,
        .flags = 0,
        .guest_phys_addr = 0,
        .memory_size = vm_conf.ram_size,
        .userspace_addr = reinterpret_cast<uint64_t>(ram_start),
    };

    r = kvmIoctl(KVM_SET_USER_MEMORY_REGION, &user_memory_region);

    if (r < 0)
        perror(("VM::" + std::string(__func__) + ": kmvIoctl").c_str());
    std::cout << "VM::" << __func__ << ": registered" << std::endl;

    return r;
}


int VM::createVcpu() {
    int r;
    vcpus = (Vcpu*)operator new[](vm_conf.vcpu_num*sizeof(Vcpu));
    std::cout << "VM::vcpus: " << vcpus << std::endl;

    for (int i = 0; i < vm_conf.vcpu_num; i++) {
        r = kvmIoctl(KVM_CREATE_VCPU, i);
        if (r < 0) {
            perror(("VM::" + std::string(__func__) + ": kvmIoctl: ").c_str());
            return -errno;
        }
        std::cout << "VM::" << __func__ << ": "
            << "fd=" << r << " cpu_id= " << i << std::endl;
        new(&vcpus[i]) Vcpu(r, kvm, i);
        std::cout << "&VM.vcpus[" << i << "]: " << &vcpus[i] << std::endl;
    }

    return 0;
}


int VM::initMachine() {
    int r;
    for (const InitMachineFunc e : initmachine_func) {
        r = (this->*e)();
        if (r < 0)
            return r;
    }
    std::cout << "VM::" << __func__ << ": success" << std::endl;
    return 0;
}


int VM::initRAM(bootloader_write_param param) {
    ebda* ebda_start = reinterpret_cast<ebda*>((
                reinterpret_cast<uint8_t*>(ram_start)+EBDA_START));
    ebda ebda_data = gen_ebda(vm_conf.vcpu_num);

    std::cout << "ebda generated" << '\n';
    std::cout << "ebda_data.fps.checksum: "
        << ebda_data.fps.checksum+0 << '\n';
    std::cout << "ebda_data.ctable.checksum: "
        << ebda_data.ctable.checksum+0 << std::endl;

    if (is_mp_checksum_valid(&ebda_data.fps)) {
        std::cout << "ebda_data.fps.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.fps.checksum corrupted" << std::endl;
    }

    if (is_mp_checksum_valid(&ebda_data.ctable)) {
        std::cout << "ebda_data.ctable.checksum is valid." << std::endl;
    } else {
        std::cerr << "ebda_data.ctable.checksum corrupted" << std::endl;
    }

    std::copy_n(&ebda_data, 1, ebda_start);
    std::cout << "ebda_data copied to guest RAM: " << ebda_start << std::endl;

    std::cout << &param << std::endl;

    std::cout << "VM::" << __func__ << ": success" << std::endl;

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
    std::cout << "IRQCHIP created" << std::endl;
    kvmIoctlCtor(KVM_CREATE_PIT2, &pit_config);
    std::cout << "PIT2 created" << std::endl;

    std::cout << "Constructed VM." << std::endl;
}


VM::~VM() {}
