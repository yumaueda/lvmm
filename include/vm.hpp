#ifndef VM_HPP
#define VM_HPP


class VM {
    public:
        VM(KVM& kvm) {
            this.kvm = kvm;
        }
    private:
        KVM& kvm;
        int vmfd;
};

#endif  // VM_HPP
