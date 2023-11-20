#include <iostream>
#include <sys/mman.h>
#include <unistd.h>

#include <linux/kvm.h>

#include <vm.hpp>
#include <vcpu.hpp>


Vcpu::Vcpu(int vcpu_fd, VM& vm, int cpu_id)
    : BaseClass(vcpu_fd), vm(vm), cpu_id(cpu_id) {
    run = static_cast<struct kvm_run*>(mmap(NULL, vm.kvm.mmap_size\
                , PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (run == MAP_FAILED) {
        std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
        std::cerr << "Vcpu.vm.kvm.mmap_size: " << vm.kvm.mmap_size << std::endl;
    } else {
        std::cout << "Vcpu.run: " << run << std::endl;
    }
}

Vcpu::~Vcpu() {
    munmap(run, vm.kvm.mmap_size);
    std::cout << "memunmapped Vcpu.run: " << run << std::endl;
    close(fd);
    std::cout << "closed Vcpu.fd: " << fd << std::endl;
}
