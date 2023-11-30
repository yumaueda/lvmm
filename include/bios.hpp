#ifndef BIOS_HPP
#define BIOS_HPP


#include <cstdint>


constexpr int      EBDA_PADDING_SIZE = 16*3;
constexpr uint32_t EBDA_START = 0x0009'fc00;
constexpr uint32_t APIC_BASE  = 0xfee0'0000;

constexpr int      MP_MAX_VCPU_NUM           = 32;

constexpr uint8_t  MP_SPEC_REV_1_4           = 4;

// _MP_
constexpr int      PARAGRAPH_SIZE            = 16;
constexpr uint32_t MPFPS_INTEL_SIGNATURE     = static_cast<uint32_t>('_')<<24|\
                                               static_cast<uint32_t>('P')<<16|\
                                               static_cast<uint32_t>('M')<< 8|\
                                               static_cast<uint32_t>('_');
constexpr uint8_t  MPFPS_LENGTH              = 1;
constexpr uint8_t  MPFPS_SPEC_REV_1_4        = MP_SPEC_REV_1_4;
constexpr uint8_t  MPFPS_FEAT_CTPRESENT      = 0;
constexpr uint8_t  MPFPS_FEAT_VIRTWIRE       = 0;

constexpr uint16_t  MPCTABLE_HEADER_LENGTH   = 44;
constexpr uint16_t  MPCTABLE_LENGTH          = MPCTABLE_HEADER_LENGTH+20*MP_MAX_VCPU_NUM;
// PCMP
constexpr uint32_t MPCTABLE_INTEL_SIGNATURE  = static_cast<uint32_t>('P')<<24|\
                                               static_cast<uint32_t>('M')<<16|\
                                               static_cast<uint32_t>('C')<< 8|\
                                               static_cast<uint32_t>('P');
constexpr uint8_t  MPCTABLE_SPEC_REV_1_4     = MP_SPEC_REV_1_4;
// SUPRMIGR
constexpr uint64_t MPCTABLE_OEM_ID           = static_cast<uint64_t>('R')<<56|\
                                               static_cast<uint64_t>('G')<<48|\
                                               static_cast<uint64_t>('I')<<40|\
                                               static_cast<uint64_t>('M')<<32|\
                                               static_cast<uint64_t>('R')<<24|\
                                               static_cast<uint64_t>('P')<<16|\
                                               static_cast<uint64_t>('U')<< 8|\
                                               static_cast<uint64_t>('S')<<0;

constexpr uint8_t  MPCTE_ENTRY_TYPE_PROC     = 0;
constexpr uint8_t  MPCTE_PROC_APIC_VER       = 0x14;
constexpr uint8_t  MPCTE_PROC_CPUFLAGS_EN    = 0b0000'0001;
constexpr uint8_t  MPCTE_PROC_CPUFLAGS_BP    = 0b0000'0010;
constexpr uint32_t MPCTE_PROC_FEATFLAGS_FPU  = \
    0b0000'0000'0000'0000'0000'0000'0000'0001;
constexpr uint32_t MPCTE_PROC_FEATFLAGS_APIC = \
    0b0000'0000'0000'0000'0000'0010'0000'0000;
// Temporary!
// We should fill the cpu signature field w/ a value returned by CPUID.
constexpr uint32_t MPCTE_PROC_CPUSIGNATURE   = \
    0b0000'0000'0000'0000'0000'0110'0000'0000;

#pragma pack(1)
struct mpfps {
    uint32_t signature = MPFPS_INTEL_SIGNATURE;
    uint32_t phys_addr_ptr;
    uint8_t  length = MPFPS_LENGTH;
    uint8_t  spec_rev = MPFPS_SPEC_REV_1_4;
    uint8_t  checksum = 0;
    uint8_t  feat_info_byte_1 = MPFPS_FEAT_CTPRESENT;
    uint8_t  feat_info_byte_2 = MPFPS_FEAT_VIRTWIRE;
    uint8_t  feat_info_byte_3 = 0;
    uint8_t  feat_info_byte_4 = 0;
    uint8_t  feat_info_byte_5 = 0;
};

#pragma pack(1)
struct mpctable_processor_entry {
    uint8_t  entry_type = MPCTE_ENTRY_TYPE_PROC;
    uint8_t  local_apic_id = 0;
    uint8_t  local_apic_ver = MPCTE_PROC_APIC_VER;
    uint8_t  cpu_flags = 0;
    uint32_t cpu_sig = 0;
    uint32_t feat_flags = MPCTE_PROC_FEATFLAGS_FPU | MPCTE_PROC_FEATFLAGS_APIC;
    uint64_t reserved = 0;
};

#pragma pack(1)
struct mpctable {
    // header
    uint32_t signature = MPCTABLE_INTEL_SIGNATURE;
    uint16_t length = MPCTABLE_LENGTH;
    uint8_t  spec_rev = MPCTABLE_SPEC_REV_1_4;
    uint8_t  checksum = 0;
    uint64_t oem_id = MPCTABLE_OEM_ID;
    uint8_t  prod_id[12] = { 0 };
    uint32_t oem_table_ptr = 0;
    uint16_t oem_table_size = 0;
    uint16_t entry_count = MP_MAX_VCPU_NUM;
    uint32_t addr_local_apic = APIC_BASE;
    uint16_t extd_table_length = 0;    // not used
    uint8_t  extd_table_checksum = 0;  // not used
    uint8_t  reserved = 0;
    // entries
    mpctable_processor_entry processor_entry[MP_MAX_VCPU_NUM];
};


template <typename MpPtr>
uint8_t mp_calc_checksum(MpPtr mpptr);


template <typename MpPtr>
uint8_t mp_gen_checksum(MpPtr mpptr) {
    return (mp_calc_checksum(mpptr) ^ 0xFF) + 0x1;
}

template <typename MpPtr>
bool is_mp_checksum_valid(MpPtr mpptr) {
    return !mp_calc_checksum(mpptr);
};


#pragma pack(1)
struct ebda {  // why do we need padding?
               // just making
               // fps.phys_addr_ptr = EBDA_START + 0x10 seems to be fine
    uint8_t  padding[EBDA_PADDING_SIZE];  // 48Bytes
    mpfps    fps;     // 16Bytes
    mpctable ctable;  // (44+20*MP_MAX_VCPU_NUM)Bytes
};

#ifdef UNITTEST
void processor_entry_init(
        mpctable_processor_entry entry_array[],
        const int vcpu_num
);
#endif  // UNITTEST

ebda gen_ebda(const int vcpu_num);


#endif  // BIOS_HPP
