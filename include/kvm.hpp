#ifndef KVM_HPP
#define KVM_HPP

#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/kvm.h>
#include <vm.hpp>


class VM;

class KVM {
    public:
        explicit KVM();

        ~KVM() {
            std::cout << "Destructing KVM..." << std::endl;
            if (this->fd >= 0) {
                close(this->fd);
                std::cout << "closed KVM.fd" << std::endl;
            }

        }

        //tss??? needed???

        int kvmCreateVM(VM** ptr_vm, uint64_t ram_size, int vcpu_num) {
            // when VM() fail we have to close vmfd in ret
            // should be done in this func? constructors raise exception. should we have to hide it inside this func?
            int ret;

            // should consider about current cpu usage
            if (vcpu_num > this->hardVcpusLimit) {
                std::cerr << "vcpu_num exceeds kvm.hardVcpusLimit!" << std::endl;
                return -1;
            } else if (vcpu_num > this->softVcpusLimit) {
                std::cout << "WARNING: vcpu_num exceeds kvm.softVcpusLimit!" << std::endl;
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

    private:
        int kvmCapCheck();

        static constexpr const char* dev_kvm = "/dev/kvm";

        // KVM state
        int fd;

        int apiVer;
        int hasImmediateExit;
        int nrSlots, nrAS;
        int softVcpusLimit, hardVcpusLimit;
};

#endif  // KVM_HPP
