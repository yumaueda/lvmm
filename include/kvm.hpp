#ifndef KVM_HPP
#define KVM_HPP

#include <string_view>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/kvm.h>

//#include <vm.hpp>


class KVM {
    public:
        explicit KVM() {
            if (this->fd = open(this->devKvm, O_RDWR) < 0)
                std::cerr << "Could not open /dev/kvm" << std::endl;

            // API ver
            this->apiVer = ioctl(this->fd, KVM_GET_API_VERSION, 0);

            if (this->apiVer < KVM_API_VERSION) {
                if (this->apiVer < 0)
                    std::cerr << "KVM_GET_API_VERSION failed" << std::endl;
                std::cerr << "KVM API version is too old: " <<  this->apiVer << std::endl;
            } else if (this->apiVer > KVM_API_VERSION) {
                std::cerr << "KVM API version is not supported: " << this->apiVer << std::endl;
            } else {
                std::cout << "KVM.apiVer: " << this->apiVer << std::endl;
            }

            // CAP check
            kvmCapCheck();
        }

        /*
        int CreateVM() {
            VM();

        }
        */

        ~KVM() {
            if (this->fd >= 0) {
                close(this->fd);
            }
        }

    private:
        void kvmCapCheck() {
            this->nrSlots = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_NR_MEMSLOTS);
            if (this->nrSlots < 0)
                this->nrSlots = 32;  // Use the default value when it's unspecified
            std::cout << "KVM.nrSlots: " << this->nrSlots << '\n';

            this->nrAS = ioctl(this->fd, KVM_CHECK_EXTENSION, KVM_CAP_MULTI_ADDRESS_SPACE);
            if (this->nrAS < 0) {
                this->nrAS = 1;
            }
            std::cout << "KVM.nrAS: " << this->nrAS << std::endl;
        }

        static constexpr const char* devKvm = "/dev/kvm";
        // KVM state
        int fd;
        int apiVer;
        int hasImmediateExit;
        int nrSlots, nrAS;
};

#endif  // KVM_HPP
