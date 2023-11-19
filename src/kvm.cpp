#include <cstring>
#include <iostream>
#include <stdexcept>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cpufeat.hpp>
#include <kvm.hpp>


int KVM::kvmCreateVM(VM** ptr_vm, uint64_t ram_size, int vcpu_num) {
    // when VM() fail we have to close vmfd in ret
    // should be done in this func? constructors raise exception. should we have to hide it inside this func?
    int ret;

    // should consider about current cpu usage
    if (vcpu_num > this->hard_vcpus_limit) {
        std::cerr << "vcpu_num exceeds kvm.hard_vcpus_limit!" << std::endl;
        return -1;
    } else if (vcpu_num > this->soft_vcpus_limit) {
        std::cout << "WARNING: vcpu_num exceeds kvm.soft_vcpus_limit!" << std::endl;
    }

    ret = ioctl(this->fd, KVM_CREATE_VM, 0);

    if (ret < 0) {
        std::cerr << "KVM_CREATE_VM failed" << std::endl;
        return -1;
    } else {
        std::cout << "KVM_CREATE_VM vmfd: " << ret << std::endl;
        std::cout << "KMV.kvmCreateVM() ram_size: " << ram_size << std::endl;
        *ptr_vm = new VM(*this, ret, ram_size, vcpu_num);
    }

    return ret;
}

void KVM::kvmCapCheck() {
    immediate_exit = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_IMMEDIATE_EXIT);
    nr_slots = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_NR_MEMSLOTS);
    nr_as = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_MULTI_ADDRESS_SPACE);
    soft_vcpus_limit = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_NR_VCPUS);
    hard_vcpus_limit = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_MAX_VCPUS);
    coalesced_mmio = kvmIoctl(KVM_CHECK_EXTENSION, KVM_CAP_COALESCED_MMIO);

    std::cout << "KVM.immediate_exit: " << immediate_exit << '\n';
    if (nr_slots == 0) {
        nr_slots = 32;
        std::cout << "KVM.nr_slots is unspecified. Using the default value: " << nr_slots << '\n';
    }

    std::cout << "KVM.nr_slots: " << nr_slots << '\n';
    if (nr_as == 0) {
        std::cout << "KVM_CAP_NR_MEMSLOTS unsupported. Assume nr_slots = 1." << '\n';
        nr_as = 1;
    }
    std::cout << "KVM.nr_as: " << nr_as << std::endl;

    if (soft_vcpus_limit == 0) {
        std::cout << "KVM_CAP_NR_VCPUS unsupported. Assume soft_vcpus_limit = 4." << '\n';
        soft_vcpus_limit = 4;
    }
    if (hard_vcpus_limit == 0) {
        std::cout << "KVM_CAP_MAX_VCPUS unsupported. Assume KVM.hard_vcpus_limit = KVM.soft_vcpus_limit." << '\n';
    }
    std::cout << "KVM.soft_vcpus_limit: " << soft_vcpus_limit << '\n';
    std::cout << "KVM.hard_vcpus_limit: " << hard_vcpus_limit << '\n';

    std::cout << "KVM.coalesced_mmio: " << coalesced_mmio << std::endl;
}


KVM::KVM() {
        std::cout << "Constructing KVM..." << std::endl;

        // Check VMX/SVM
        if (!cpuSupportsVM())
            throw std::runtime_error(std::string(__func__) + ": VMX/SVM unsupported.");
        std::cout << "VMX/SVM detected." << std::endl;

        // Open sysfd
        if ((fd = open(dev_kvm, O_RDWR)) < 0)
            throw std::runtime_error(std::string(__func__) + ": Could not open " + dev_kvm + ".");
        std::cout << "KVM.fd: " << fd << std::endl;

        // API version check
        std::cout << "KVM_API_VERSION from linux header: " << KVM_API_VERSION << std::endl;
        api_ver = ioctl(fd, KVM_GET_API_VERSION, 0);

        if (api_ver < KVM_API_VERSION)
            if (api_ver < 0)
                throw std::runtime_error(std::string(__func__) + ": ioctl KVM_GET_API_VERSION failed.");
            else
                throw std::runtime_error(std::string(__func__) + ": KVM API version " + std::to_string(api_ver) + " is too old.");
        else if (api_ver > KVM_API_VERSION)
                throw std::runtime_error(std::string(__func__) + ": KVM API version " + std::to_string(api_ver) + " is not supported.");
        else
            std::cout << "KVM.api_ver: " << api_ver << std::endl;

        // Cap check
        kvmCapCheck();  // there should be return value and exception system


        mmap_size = ioctl(fd, KVM_GET_VCPU_MMAP_SIZE, 0);
        if (mmap_size < 0)
        std::cout << "mmap_size: " << mmap_size << std::endl;
}

KVM::~KVM() {
    std::cout << "Destructing KVM..." << std::endl;
    if (this->fd >= 0) {
        close(this->fd);
        std::cout << "closed KVM.fd" << std::endl;
    }

}
