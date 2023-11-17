#include <iostream>
#include <stdexcept>

#include <kvm.hpp>
#include <cpufeat.hpp>

int KVM::kvmCapCheck() {
    this->hasImmediateExit = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_IMMEDIATE_EXIT);
    std::cout << "KVM.hasImmediateExit: " << this->hasImmediateExit << '\n';

    this->nrSlots = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_NR_MEMSLOTS);
    if (this->nrSlots < 0) {
        std::cout << "KVM.nrSlots is unspecified. Using the defaul value." << this->nrSlots << '\n';
        this->nrSlots = 32;  // Use the default value when it's unspecified
    }
    std::cout << "KVM.nrSlots: " << this->nrSlots << '\n';

    this->nrAS = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_MULTI_ADDRESS_SPACE);
    if (this->nrAS < 0) {
        this->nrAS = 1;
    }
    std::cout << "KVM.nrAS: " << this->nrAS << std::endl;

    this->softVcpusLimit = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_NR_VCPUS);
    if (this->softVcpusLimit < 0) {
        std::cerr << "KVM_CHECK_EXTENSION, KVM_CAP_NR_VCPUS failed. Assume softVcpusLimit==4." << std::endl;
        this->softVcpusLimit = 4;
    }
    this->hardVcpusLimit = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_MAX_VCPUS);
    if (this->hardVcpusLimit < 0) {
        std::cerr << "KVM_CHECK_EXTENSION, KVM_CAP_MAX_VCPUS failed. Assume KVM.hardVcpusLimit==KVM.softVcpusLimit." << std::endl;
    }
    std::cout << "KVM.softVcpusLimit: " << this->softVcpusLimit << std::endl;
    std::cout << "KVM.hardVcpusLimit: " << this->hardVcpusLimit << std::endl;
    return 0;
}


KVM::KVM() {
        int r;

        std::cout << "Constructing KVM..." << std::endl;

        // Check VMX/SVM
        if (!cpuSupportsVM())
            throw std::runtime_error(std::string(__func__) + "VMX/SVM unsupported.");
        std::cout << "VMX/SVM detected." << std::endl;

        if ((fd = open(dev_kvm, O_RDWR)) < 0)
            throw std::runtime_error(std::string(__func__) + "Could not open " + dev_kvm);
        std::cout << "KVM.fd: " << fd << std::endl;

        // API ver
        std::cout << "KVM_API_VERSION from linux header: " << KVM_API_VERSION << std::endl;
        apiVer = ioctl(this->fd, KVM_GET_API_VERSION, 0);

        if (this->apiVer < KVM_API_VERSION) {
            if (this->apiVer < 0)
                std::cerr << "ioctl KVM_GET_API_VERSION failed" << std::endl;
            else
                std::cerr << "KVM API version is too old: " <<  this->apiVer << std::endl;
        } else if (this->apiVer > KVM_API_VERSION) {
            std::cerr << "KVM API version is not supported: " << this->apiVer << std::endl;
        } else {
            std::cout << "KVM.apiVer: " << this->apiVer << std::endl;
        }

        // CAP check
        r = kvmCapCheck();  // there should be return value and exception system
        std::cout << "tmp kvmcapcheck: " << r << std::endl;


        r = ioctl(this->fd, KVM_GET_VCPU_MMAP_SIZE, 0);
        std::cout << "mmap_size: " << r << std::endl;
}
