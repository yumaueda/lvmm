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
        explicit KVM() {
            int r;

            std::cout << "Constructing KVM..." << std::endl;

            // Check VMX/SVM here!

            if ((this->fd = open(this->devKvm, O_RDWR)) < 0) {
                std::cerr << "Could not open /dev/kvm" << std::endl;
                // throw an exception here!
            }
            std::cout << "KVM.fd: " << this->fd << std::endl;

            // API ver
            std::cout << "KVM_API_VERSION from linux header: " << KVM_API_VERSION << std::endl;
            this->apiVer = ioctl(this->fd, KVM_GET_API_VERSION, 0);

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
        }


        ~KVM() {
            std::cout << "Destructing KVM..." << std::endl;
            if (this->fd >= 0) {
                close(this->fd);
                std::cout << "closed KVM.fd" << std::endl;
            }

        }

        //tss??? needed???
        //getpagesize

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

        int kvmCapCheck() {
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



        static constexpr const char* devKvm = "/dev/kvm";

        // KVM state
        int fd;

        int apiVer;
        int hasImmediateExit;
        int nrSlots, nrAS;
        int softVcpusLimit, hardVcpusLimit;
};

#endif  // KVM_HPP
