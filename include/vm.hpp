/*
 *  include/vm.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_VM_HPP_
#define INCLUDE_VM_HPP_


#include <linux/kvm.h>

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>

#include <baseclass.hpp>
#include <boot.hpp>
#include <kvm.hpp>
#include <vcpu.hpp>


class Vcpu;
class VM;
class KVM;


constexpr const int INITMACHINE_FUNC_NUM = 3;


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

typedef int (VM::*InitMachineFunc)();


class VM : public BaseClass {
 public:
    explicit VM(int vm_fd, KVM* kvm, vm_config config);
    ~VM();

    int initMachine();
    int initRAM(std::string cmdline);
    int initVcpuRegs();
    int createPageTable(uint64_t boot_pgtable_base, bool is_64bit_boot);
    int initVcpuSregs(bool is_64bit);

 private:
    KVM* kvm;
    const vm_config vm_conf;
    std::ifstream kernel, initramfs;

    static constexpr const kvm_pit_config pit_config = {
        .flags = KVM_PIT_SPEAKER_DUMMY,
        .pad = {0},
    };

    const InitMachineFunc initmachine_func[INITMACHINE_FUNC_NUM] = {
        &VM::allocGuestRAM,
        &VM::setUserMemRegion,
        &VM::createVcpu,
    };

    Vcpu* vcpus = static_cast<Vcpu*>(nullptr);
    void* ram_start = nullptr;
    kvm_userspace_memory_region user_memory_region;  // TMP

    // initMachine()
    int allocGuestRAM();
    int setUserMemRegion();
    int createVcpu();
};


#endif  // INCLUDE_VM_HPP_
