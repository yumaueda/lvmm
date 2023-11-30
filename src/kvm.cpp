#include <cerrno>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <cpufeat.hpp>
#include <kvm.hpp>


int KVM::kvmCreateVM(VM** ptr_vm, vm_config vm_conf) {
    int r;

    // should consider about current cpu usage
    if (vm_conf.vcpu_num > cap.hard_vcpus_limit) {
        std::cerr << "vm_conf.vcpu_num exceeds kvm.hard_vcpus_limit!" << std::endl;
        return -1;
    } else if (vm_conf.vcpu_num > cap.soft_vcpus_limit) {
        std::cout << "WARNING: vm_conf.vcpu_num exceeds kvm.soft_vcpus_limit!" << std::endl;
    }

    r = kvmIoctl(KVM_CREATE_VM, 0);

    if (r < 0) {
        std::cerr << "KVM_CREATE_VM failed." << std::endl;
        return -1;
    } else {
        std::cout << "KVM_CREATE_VM fd: " << r << std::endl;
        *ptr_vm = new VM(r, *this, vm_conf);
    }

    return r;
}

int KVM::kvmCapCheck() {
    int* kvm_cap_ptr = reinterpret_cast<int*>(&cap);

    for (const int e : KVM_CAP_CHECK)
        *kvm_cap_ptr++ = kvmIoctlCtor(KVM_CHECK_EXTENSION, e);

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

    !cpuSupportsVM();
    api_ver = kvmIoctlCtor(KVM_GET_API_VERSION, 0);
    kvmCapCheck();
    mmap_size = kvmIoctlCtor(KVM_GET_VCPU_MMAP_SIZE, 0);

    std::cout << "VMX/SVM detected." << std::endl;
    std::cout << "KVM.api_ver: " << api_ver << std::endl;
    std::cout << "KVM.mmap_size: " << mmap_size << std::endl;

    std::cout << "Constructed KVM." << std::endl;
}

KVM::~KVM() {}
