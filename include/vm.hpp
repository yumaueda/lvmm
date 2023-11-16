#ifndef VM_HPP
#define VM_HPP

#include <cerrno>
#include <cstring>

#include <sys/mman.h>
#include <unistd.h>

#include <linux/kvm.h>
#include <kvm.hpp>


// tmp const!
#define KVM_32BIT_GAP_START

class KVM;

class VM {
    public:
        explicit VM(KVM& kvm, int vmfd, const uint64_t ram_size, const int vcpu_num) : kvm(kvm), vmfd(vmfd), ram_size(ram_size), vcpu_num(vcpu_num) {
            std::cout << "Constructing VM..." << std::endl;
            errno = 0;
            int r;

            /*
             * Create VIOAPIC, VPIC. Set up future vCPUs to use them as a local APIC.
             * GSI00-15 -> IOAPIC/PIC
             * GIS16-23 -> IOAPIC
             */
            r = ioctl(this->vmfd, KVM_CREATE_IRQCHIP);
            if (r < 0)
                std::cerr << "KVM_CREATE_IRQCHIP failed" << std::endl;
                // exception!
            r = ioctl(this->vmfd, KVM_CREATE_PIT2, &this->pit_config);
            if (r < 0)
                std::cerr << "KVM_CREATE_PIT2 failed" << std::endl;
                // exception!

            // RAM
            // not caring about hugetlbpage
            this->ram_start = NULL;
            this->ram_page_size = getpagesize();
            // FIX? 32_BIT_GAP and mprotect(PROT_NONE)?
            this->ram_start = mmap(NULL, this->ram_size, (PROT_READ|PROT_WRITE), (MAP_PRIVATE|MAP_ANONYMOUS|MAP_NORESERVE), -1, 0);
            if (this->ram_start == MAP_FAILED) {
                std::cerr << "mmap failed: " << std::strerror(errno) << std::endl;
                std::cerr << "VM.ram_size: " << this->ram_size << std::endl;
                // exception!
            } else {
                errno = 0;
                if (madvise(this->ram_start, this->ram_size, MADV_MERGEABLE) < 0)
                    std::cerr << "madvise failed: " << std::strerror(errno) << std::endl;
            }


            // vCPU(s)

        }

        ~VM() {
            errno = 0;
            std::cout << "Destructing VM..." << std::endl;
            if (this->vmfd >= 0) {
                close(this->vmfd);
                std::cout << "closed VM.vmfd" << std::endl;
            }
            if (this->ram_start != NULL) {
                if (munmap(this->ram_start, this->ram_size) < 0)
                    std::cerr << "munmap VM.ram_start failed: " << std::strerror(errno) << std::endl;
                else
                    std::cout << "munmapped VM.ram_start" << std::endl;
            }
        }
    private:
        KVM& kvm;
        const int vmfd;
        const struct kvm_pit_config pit_config = {.flags = KVM_PIT_SPEAKER_DUMMY, .pad = {0}};
        const uint64_t ram_size;  // in bytes
        void* ram_start;
        int ram_page_size;
        const int vcpu_num;
};

#endif  // VM_HPP
