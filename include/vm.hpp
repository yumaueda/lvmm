#ifndef VM_HPP
#define VM_HPP


#include <linux/kvm.h>

#include <kvm.hpp>
#include <vcpu.hpp>


// tmp const!
#define KVM_32BIT_GAP_START


class KVM;
class Vcpu;


class VM {
    public:
        explicit VM(KVM& kvm, int vmfd, const uint64_t ram_size, const int vcpu_num);
        ~VM();

        KVM& kvm;

    private:
        Vcpu **vcpus;
        const int vmfd;
        static constexpr const struct kvm_pit_config pit_config = {.flags = KVM_PIT_SPEAKER_DUMMY, .pad = {0}};
        const uint64_t ram_size;  // in bytes
        void* ram_start;
        int ram_page_size;
        const int vcpu_num;
};

#endif  // VM_HPP
