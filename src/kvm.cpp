#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <cpufeat.hpp>
#include <kvm.hpp>


int KVM::kvmCreateVM(VM** ptr_vm, vm_config vm_conf) {
    // when VM() fail we have to close vmfd in ret
    // should be done in this func? constructors raise exception. should we have to hide it inside this func?
    int ret;

    // should consider about current cpu usage
    if (vm_conf.vcpu_num > cap.hard_vcpus_limit) {
        std::cerr << "vm_conf.vcpu_num exceeds kvm.hard_vcpus_limit!" << std::endl;
        return -1;
    } else if (vm_conf.vcpu_num > cap.soft_vcpus_limit) {
        std::cout << "WARNING: vm_conf.vcpu_num exceeds kvm.soft_vcpus_limit!" << std::endl;
    }

    ret = kvmIoctl(KVM_CREATE_VM, 0);

    if (ret < 0) {
        std::cerr << "KVM_CREATE_VM failed" << std::endl;
        return -1;
    } else {
        std::cout << "KVM_CREATE_VM fd: " << ret << std::endl;
        *ptr_vm = new VM(ret, *this, vm_conf);
    }

    return ret;
}

int KVM::kvmCapCheck() {
    int* kvm_cap_ptr = reinterpret_cast<int*>(&cap);

    for (const int e : KVM_CAP_CHECK) {
        *kvm_cap_ptr = kvmIoctl(KVM_CHECK_EXTENSION, e);
        if (*kvm_cap_ptr < 0) {
            std::cerr
                << "kvmCapCheck failed: KVM_CAP_CHECK[" << e << "]"
                << std::endl;
            return *kvm_cap_ptr;
        }
        kvm_cap_ptr++;
    }

    if (cap.nr_slots == 0) {
        cap.nr_slots = 32;
        std::cout << "KVM.nr_slots is unspecified. Using the default value: "
            << cap.nr_slots << '\n';
    }

    if (cap.nr_as == 0) {
        std::cout << "KVM_CAP_NR_MEMSLOTS unsupported."
            << " Assume nr_slots = 1."<< '\n';
        cap.nr_as = 1;
    }

    if (cap.soft_vcpus_limit == 0) {
        std::cout << "KVM_CAP_NR_VCPUS unsupported."
            << " Assume soft_vcpus_limit = 4." << '\n';
        cap.soft_vcpus_limit = 4;
    }

    if (cap.hard_vcpus_limit == 0) {
        std::cout << "KVM_CAP_MAX_VCPUS unsupported. "
            << "Assume KVM.hard_vcpus_limit = KVM.soft_vcpus_limit." << '\n';
    }

    return EXIT_SUCCESS;
}


KVM::KVM(int fd) : BaseClass(fd) {
        std::cout << "Constructing KVM..." << std::endl;

        // Check VMX/SVM
        if (!cpuSupportsVM())
            throw std::runtime_error(
                    std::string(__func__) + ": VMX/SVM unsupported.");
        std::cout << "VMX/SVM detected." << std::endl;

        // API version check
        std::cout << "KVM_API_VERSION from linux header: "
            << KVM_API_VERSION << std::endl;
        api_ver = kvmIoctl(KVM_GET_API_VERSION, 0);

        if (api_ver < KVM_API_VERSION)
            if (api_ver < 0)
                throw std::runtime_error(
                        std::string(__func__) +
                        ": ioctl KVM_GET_API_VERSION failed.");
            else
                throw std::runtime_error(std::string(__func__) +
                        ": KVM API version " +
                        std::to_string(api_ver) +
                        " is too old.");
        else if (api_ver > KVM_API_VERSION)
                throw std::runtime_error(std::string(__func__) +
                        ": KVM API version " +
                        std::to_string(api_ver) +
                        " is not supported.");
        else
            std::cout << "KVM.api_ver: " << api_ver << std::endl;

        // should raise exception if anything goes wrong
        kvmCapCheck();


        mmap_size = kvmIoctl(KVM_GET_VCPU_MMAP_SIZE, 0);
        std::cout << "mmap_size: " << mmap_size << std::endl;
}

KVM::~KVM() {
    std::cout << "Destructing KVM..." << std::endl;
    if (fd >= 0) {
        close(fd);
        std::cout << "closed KVM.fd" << std::endl;
    }

}
