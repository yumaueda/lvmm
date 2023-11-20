#include <cstring>
#include <cerrno>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <vm.hpp>


VM::VM(int vm_fd, KVM& kvm, const uint64_t ram_size, const int vcpu_num)\
        : BaseClass(vm_fd), kvm(kvm), ram_size(ram_size), vcpu_num(vcpu_num)
{
    std::cout << "Constructing VM..." << std::endl;
    vcpus = NULL;
    errno = 0;
    int r;

    // Currently we only assumes the case where unrestricted_guest == 1
    // SET_TSS
    // SET_IDENTITY_MAP

    /*
     * Create VIOAPIC, VPIC.
     * Set up future vCPUs to use them as a local APIC.
     * GSI00-15 -> IOAPIC/PIC
     * GIS16-23 -> IOAPIC
     */
    r = kvmIoctl(KVM_CREATE_IRQCHIP);
    if (r < 0)
        std::cerr << "KVM_CREATE_IRQCHIP failed" << std::endl;
        // exception!
    r = kvmIoctl(KVM_CREATE_PIT2, &this->pit_config);
    if (r < 0)
        std::cerr << "KVM_CREATE_PIT2 failed" << std::endl;
        // exception!

    // RAM
    // not caring about hugetlbpage
    ram_start = NULL;
    ram_page_size = getpagesize();
    // FIX? 32_BIT_GAP and mprotect(PROT_NONE)?
    ram_start = mmap(NULL, ram_size, (PROT_READ|PROT_WRITE)\
            , (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);
    if (ram_start == MAP_FAILED) {
        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
        std::cerr << "VM.ram_size: " << ram_size << std::endl;
        // exception!
    } else {
        std::cout << "RAM mapped. VM.ram_start: " << ram_start << std::endl;
        if (madvise(ram_start, ram_size, MADV_MERGEABLE) < 0)
            std::cerr << "madvise failed: "\
                << std::strerror(errno) << std::endl;
    }


    // vCPU(s)
    // some kind of kvm_vcpu struct is needed. will be implemented soon
    vcpus = new Vcpu*[vcpu_num];

    for (int i = 0; i < vcpu_num; i++) {
        r = kvmIoctl(KVM_CREATE_VCPU, i);
        if (r < 0) {
            std::cerr << "KVM_CREATE_VCPU failed" << std::endl;
            // exception
        } else {
            std::cout << "KVM_CREATE_VCPU cpuid: " << i << " vcpufd: " << r << std::endl;
            vcpus[i] = new Vcpu(r, *this, i);
        }
    }


}

VM::~VM() {
    errno = 0;
    std::cout << "Destructing VM..." << std::endl;
    for (int i = 0; i < this->vcpu_num; i++) {
        std::cout << "deleted VM.vcpus[i]" << std::endl;
        delete vcpus[i];
    }
    if (vcpus != NULL) {
        std::cout << "deleted VM.vcpus" << std::endl;
        delete vcpus;
    }
    if (this->ram_start != NULL) {
        if (munmap(this->ram_start, this->ram_size) < 0)
            std::cerr << "munmap VM.ram_start failed: " << std::strerror(errno) << std::endl;
        else
            std::cout << "munmapped VM.ram_start" << std::endl;
    }
    if (this->fd >= 0) {
        close(this->fd);
        std::cout << "closed VM.fd" << std::endl;
    }

}
