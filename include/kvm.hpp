#ifndef KVM_HPP
#define KVM_HPP

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/kvm.h>
#include <baseclass.hpp>
#include <vm.hpp>


struct vm_config;
class VM;


constexpr const char* DEV_KVM = "/dev/kvm";


struct kvm_cap {
    int immediate_exit;
    int nr_slots;
    int nr_as;
    int soft_vcpus_limit;
    int hard_vcpus_limit;
    int coalesced_mmio;
};

class KVM : public BaseClass {
    public:
        explicit KVM(int fd);
        ~KVM();

        int mmap_size;

        static int getKVMFD(){
            int r;
            if ((r = open(DEV_KVM, O_RDWR)) < 0) {
                std::cout<< (std::string(__func__) + ": Could not open " + DEV_KVM + ".") << std::endl;
                return -errno;
            }
            return r;
        };

        int kvmCreateVM(VM** ptr_vm, vm_config vm_conf);

    private:
        int api_ver;
        kvm_cap cap;

        void kvmCapCheck();
};


#endif  // KVM_HPP
