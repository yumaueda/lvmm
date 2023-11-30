#ifndef VM_HPP
#define VM_HPP


#include <cstdint>
#include <linux/kvm.h>
#include <baseclass.hpp>
#include <kvm.hpp>
#include <vcpu.hpp>


class KVM;
class Vcpu;


struct vm_config {
    const int vcpu_num;
    const int padding = 0;
    const uint64_t ram_size;  // in bytes
    const char *kernel_path;
    const char *initramfs_path;
    /*
     * padding: 
     *   I don't know why, but without padding,
     *   the offset of the vm_config structure member
     *   after vcpu_num in the VM class is accessed with 4Byte less
     */
};

class VM : public BaseClass {
    public:
        explicit VM(int vm_fd, KVM& kvm, vm_config config);
        ~VM();

        int allocGuestRAM();
        int setUserMemRegion();
        void createVcpu();

    private:
        KVM& kvm;
        const vm_config vm_conf;

        static constexpr const kvm_pit_config pit_config = {
            .flags = KVM_PIT_SPEAKER_DUMMY,
            .pad = {0},
        };

        Vcpu* vcpus = static_cast<Vcpu*>(nullptr);
        void* ram_start = nullptr;
        kvm_userspace_memory_region user_memory_region;  // TMP
};


#endif  // VM_HPP
