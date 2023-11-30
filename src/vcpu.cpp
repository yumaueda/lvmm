#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/kvm.h>

#include <kvm.hpp>
#include <vcpu.hpp>


Vcpu::Vcpu(int vcpu_fd, KVM& kvm, int cpu_id)
    : BaseClass(vcpu_fd), kvm(kvm), cpu_id(cpu_id) {
    std::cout << "Constructing Vcpu..." << std::endl;

    run = static_cast<struct kvm_run*>(mmap(NULL, kvm.mmap_size
                , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (run == MAP_FAILED) {
        perror("Vcpu.run: mmap");
        // The type of exception should be detailed.
        throw std::runtime_error("Vcpu::" + std::string(__func__)
                + ": " + strerror(errno));
    } else {
        std::cout << "Vcpu.run mmaped: " << run << std::endl;
    }

    std::cout << "Constructed Vcpu." << std::endl;
}

Vcpu::~Vcpu() {}
