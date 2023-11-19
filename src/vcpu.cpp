#include <iostream>
#include <unistd.h>
#include <vm.hpp>
#include <vcpu.hpp>


Vcpu::Vcpu(VM& vm, int cpu_id, int vcpu_fd) : vm(vm), cpu_id(cpu_id), vcpu_fd(vcpu_fd) {
    dummy = new int;
}

Vcpu::~Vcpu() {
    close(this->vcpu_fd);
    std::cout << "closed vcpu_fd: " << this->vcpu_fd << std::endl;
    delete dummy;
}
