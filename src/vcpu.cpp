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
        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
    } else {
        std::cout << "Vcpu.run(kvm_run) mapped: fd=" << fd
                  << " mmap_size=" << kvm.mmap_size << std::endl;
        std::cout << "Vcpu.run: " << run << std::endl;
    }
    std::cout << "Constructed Vcpu" << std::endl;
}

Vcpu::~Vcpu() {
    std::cout << "Destructing Vcpu..." << std::endl;
    munmap(run, kvm.mmap_size);
    std::cout << "memunmapped Vcpu.run: " << run << std::endl;
    close(fd);
    std::cout << "closed Vcpu.fd: " << fd << std::endl;
    std::cout << "Destructed Vcpu" << std::endl;
}
