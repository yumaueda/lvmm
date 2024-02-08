/*
 *  include/kvm.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_KVM_HPP_
#define INCLUDE_KVM_HPP_


#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/kvm.h>

#include <cstdio>
#include <cerrno>
#include <iostream>
#include <string>

#include <baseclass.hpp>
#include <vm.hpp>


struct vm_config;
class VM;


constexpr const char* DEV_KVM = "/dev/kvm";

constexpr const int KVM_CAP_CHECK[] = {
    KVM_CAP_IMMEDIATE_EXIT,
    KVM_CAP_NR_MEMSLOTS,
    KVM_CAP_MULTI_ADDRESS_SPACE,
    KVM_CAP_NR_VCPUS,
    KVM_CAP_MAX_VCPUS,
    KVM_CAP_COALESCED_MMIO,
};


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

    static int getKVMFD() {
        int r;

        if ((r = open(DEV_KVM, O_RDWR)) < 0) {
            perror(("KVM::" + std::string(__func__) + ": open").c_str());
            return -errno;
        }
        std::cout << "KVM::" << __func__ << ": " << r << std::endl;

        return r;
    }

    int kvmCreateVM(VM** ptr_vm, vm_config vm_conf);
    int getSupportedCPUID(kvm_cpuid2* kvm_cpuid);

 private:
    int api_ver;
    kvm_cap cap;

    int kvmCapCheck();
};


#endif  // INCLUDE_KVM_HPP_
