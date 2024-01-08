/*
 *  include/cpufeat.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_CPUFEAT_HPP_
#define INCLUDE_CPUFEAT_HPP_


#include <cstdint>
#include <stdexcept>
#include <string>


// Intel
#define VIS_INTEL_EBX        0x756e6547 /* "Genu" */
#define VIS_INTEL_EDX        0x49656e69 /* "ineI" */
#define VIS_INTEL_ECX        0x6c65746e /* "ntel" */
#define INTEL_EAX_FEAT       0x00000001
#define INTEL_FEAT_VMX_SHIFT 5


// AMD
#define VIS_AMD_EBX        0x68747541 /* "Auth" */
#define VIS_AMD_EDX        0x69746e65 /* "enti" */
#define VIS_AMD_ECX        0x444d4163 /* "cAMD" */
#define AMD_EAX_FEAT       0x80000001
#define AMD_FEAT_SVM_SHIFT 2


struct cpuid_regs {
    uint32_t eax, ebx, ecx, edx;
};

static inline void cpuid(cpuid_regs* regs) {
    asm volatile("cpuid" :
            "=a" (regs->eax),
            "=b" (regs->ebx),
            "=c" (regs->ecx),
            "=d" (regs->edx) :
             "0" (regs->eax), "2" (regs->ecx));
}

bool cpuSupportsVM() {
    // NOT sure this works on AMD.
    cpuid_regs regs = { 0, 0, 0, 0 };
    uint32_t eax_feat;
    int feat_vm_shift;

    cpuid(&regs);

    switch (regs.ebx) {
        case VIS_INTEL_EBX:
            if (regs.ecx == VIS_INTEL_ECX && regs.edx == VIS_INTEL_EDX) {
                eax_feat = INTEL_EAX_FEAT;
                feat_vm_shift = INTEL_FEAT_VMX_SHIFT;
                break;
            } else {
                throw std::runtime_error(std::string(__func__) + ": "
                        "Invalid VIS returned. "
                        "regs.ebx is from Intel, but others are not.");
                return false;
            }
        case VIS_AMD_EBX:
            if (regs.ecx == VIS_AMD_ECX && regs.edx == VIS_AMD_EDX) {
                eax_feat = AMD_EAX_FEAT;
                feat_vm_shift = AMD_FEAT_SVM_SHIFT;
                break;
            } else {
                throw std::runtime_error(std::string(__func__) + ": "
                        "Invalid VIS returned. "
                        "regs.ebx is from AMD, but others are not.");
                return false;
            }
            break;
        default:
            throw std::runtime_error(std::string(__func__) + ": "
                    "VIS couldn't be identified returned.");
            return false;
    }

    if (regs.eax < eax_feat) {
        throw std::runtime_error(std::string(__func__) + ": "
                "Getting feature information is not supported.");
        return false;
    }


    regs = { eax_feat, 0, 0, 0 };
    cpuid(&regs);

    return regs.ecx & 1 << feat_vm_shift;
}


#endif  // INCLUDE_CPUFEAT_HPP_
