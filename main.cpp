#include <iostream>

#include <kvm.hpp>

int main(void) {
    int r;
    KVM *kvm = new KVM;
    VM *vm;

    const uint64_t ram_size = static_cast<uint64_t>(8) << 30;
    const int vcpu_num = 2;

    std::cout << "main() ram_size: " << ram_size << std::endl;
    r = kvm->kvmCreateVM(&vm, ram_size, vcpu_num);
    if (r < 0) {
        std::cerr << "kvm->kvmCreateVM() failed" << std::endl;
        goto out_tmp;
    }


    delete vm;
    delete kvm;
    return 0;

    delete vm;
out_tmp:
    delete kvm;
    return -1;
}
