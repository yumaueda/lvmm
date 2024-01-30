/*
 *  include/boot.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_BOOT_HPP_
#define INCLUDE_BOOT_HPP_


#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include <vm.hpp>


class VM;


constexpr int      PARAGRAPH_SIZE      = 16;
constexpr int      SECT_SIZE           = 512;

constexpr uint64_t REALMODE_IVT_START  = 0x0000'0000;
constexpr uint64_t BOOT_PARAMS_ADDR    = 0x0001'0000;
constexpr uint64_t COMMANDLINE_ADDR    = 0x0002'0000;
constexpr uint64_t BOOT_PAGETABLE_BASE = 0x0003'0000;
constexpr uint64_t EBDA_START          = 0x0009'fc00;
constexpr uint64_t VGARAM_START        = 0x000a'0000;
constexpr uint64_t MBBIOS_START        = 0x000f'0000;
constexpr uint64_t HIGHMEM_BASE        = 0x0010'0000;
constexpr uint64_t INITRAMFS_ADDR      = 0x0f00'0000;
constexpr uint64_t TSS_BASE            = 0xf800'0000;
constexpr uint64_t IDENTITY_MAP_BASE   = 0xf800'3000;
constexpr uint64_t APIC_BASE           = 0xfee0'0000;
constexpr uint32_t MBBIOS_SIZE         = 0x0000'ffff;

constexpr int      EBDA_PADDING_SIZE   = 16*3;


constexpr int      MP_MAX_VCPU_NUM           = 32;
constexpr uint8_t  MP_SPEC_REV_1_4           = 4;
constexpr uint32_t MPFPS_INTEL_SIGNATURE     = static_cast<uint32_t>('_') << 24
                                             | static_cast<uint32_t>('P') << 16
                                             | static_cast<uint32_t>('M') << 8
                                             | static_cast<uint32_t>('_') << 0;
constexpr uint8_t  MPFPS_LENGTH              = 1;
constexpr uint8_t  MPFPS_SPEC_REV_1_4        = MP_SPEC_REV_1_4;
constexpr uint8_t  MPFPS_FEAT_CTPRESENT      = 0;
constexpr uint8_t  MPFPS_FEAT_VIRTWIRE       = 0;
constexpr uint32_t MPCTABLE_INTEL_SIGNATURE  = static_cast<uint32_t>('P') << 24
                                             | static_cast<uint32_t>('M') << 16
                                             | static_cast<uint32_t>('C') << 8
                                             | static_cast<uint32_t>('P') << 0;
constexpr uint8_t  MPCTABLE_SPEC_REV_1_4     = MP_SPEC_REV_1_4;
constexpr uint64_t MPCTABLE_OEM_ID           = static_cast<uint64_t>('T') << 56
                                             | static_cast<uint64_t>('S') << 48
                                             | static_cast<uint64_t>('E') << 40
                                             | static_cast<uint64_t>('T') << 32
                                             | static_cast<uint64_t>('G') << 24
                                             | static_cast<uint64_t>('I') << 16
                                             | static_cast<uint64_t>('M') << 8
                                             | static_cast<uint64_t>('L') << 0;
constexpr uint8_t  MPCTE_ENTRY_TYPE_PROC     = 0;
constexpr uint8_t  MPCTE_PROC_APIC_VER       = 0x14;
constexpr uint8_t  MPCTE_PROC_CPUFLAGS_EN    = 0b0000'0001;
constexpr uint8_t  MPCTE_PROC_CPUFLAGS_BP    = 0b0000'0010;
constexpr uint32_t MPCTE_PROC_FEATFLAGS_FPU  = 0x0000'0001;
constexpr uint32_t MPCTE_PROC_FEATFLAGS_APIC = 0x0000'0200;
// Temporary!
// We should fill the cpu signature field w/ a value returned by CPUID.
constexpr uint32_t MPCTE_PROC_CPUSIGNATURE   = 0x0000'0600 << 16;

constexpr uint32_t BOOT_EDD_MBR_SIG_MAX    = 16;
constexpr uint32_t BOOT_E820_MAP_MAX       = 128;
constexpr uint32_t BOOT_E820_TYPE_RAM      = 1;
constexpr uint32_t BOOT_E820_TYPE_RESERVED = 2;

constexpr int      SETUP_HEADER_ADDR       = 0x0000'01f1;
constexpr uint16_t BOOT_HDR_VID_MODE_NML   = 0xffff;
constexpr uint16_t BOOT_HDR_BOOT_FLAG      = 0xaaff;
constexpr uint32_t BOOT_HDR_MAGIC          = static_cast<uint32_t>('S') << 24
                                           | static_cast<uint32_t>('r') << 16
                                           | static_cast<uint32_t>('d') << 8
                                           | static_cast<uint32_t>('H') << 0;
constexpr uint8_t  BOOT_HDR_BLT_UNDEFINED  = 0xff;
constexpr uint8_t  BOOT_HDR_LF_HIGH        = 0b0000'0001;  // R
constexpr uint8_t  BOOT_HDR_LF_KASLR       = 0b0000'0010;  // KI
constexpr uint8_t  BOOT_HDR_LF_QUIET       = 0b0001'0000;  // W  early msg off
constexpr uint8_t  BOOT_HDR_LF_KEEP_SGMT   = 0b0100'0000;  // Ob 2.07+ obsolute
constexpr uint8_t  BOOT_HDR_LF_HEAP        = 0b1000'0000;  // W
constexpr uint32_t BOOT_HDR_HEAPEND_OFFSET = 0x200;

constexpr int      ELF_MAGIC_SIZE            = 4;
constexpr char     ELF_MAGIC[ELF_MAGIC_SIZE] = {0x7f, 'E', 'L', 'F'};


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
    uint8_t  entry_type = 0;
    uint8_t  local_apic_id = 0;
    uint8_t  local_apic_ver = 0;
    uint8_t  cpu_flags = 0;
    uint32_t cpu_sig = 0;
    uint32_t feat_flags = 0;
    uint64_t reserved = 0;
};

#pragma pack(1)
struct mpctable {
    // header
    uint32_t signature = MPCTABLE_INTEL_SIGNATURE;
    uint16_t length = sizeof(mpctable);
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

#pragma pack(1)
struct ebda {  // why do we need padding?
               // just making
               // fps.phys_addr_ptr = EBDA_START + 0x10 seems to be fine
    uint8_t  padding[EBDA_PADDING_SIZE] = { 0 };  // 48Bytes
    mpfps    fps;     // 16Bytes
    mpctable ctable;  // (44+20*MP_MAX_VCPU_NUM)Bytes
};

#pragma pack(1)
struct setup_header {
    // *: fields should be written from bootloader
    uint8_t  setup_sects = 0;            //  R     ALL
    uint16_t root_flags = 0;             //  Mop   ALL       deprecated
    uint32_t sys_size = 0;               //  R     2.04+/ALL
    uint16_t ram_size = 0;               //  KI    ALL       obsolete
    uint16_t vid_mode = 0;               // *Mob   ALL
    uint16_t root_dev = 0;               //  Mop   ALL       deprecated
    uint16_t boot_flag = 0;              //  R     ALL
    uint16_t jump = 0;                   //  R     2.00+
    uint32_t header = 0;                 //  R     2.00+
    uint16_t version = 0;                //  R     2.00+
    uint32_t realmode_swtch = 0;         //  Mop   2.00+
    uint16_t start_sys_seg = 0;          //  R     2.00+     obsolete
    uint16_t kernel_version = 0;         //  R     2.00+
    uint8_t  type_of_loader = 0;         // *Wob   2.00+
    uint8_t  loadflags = 0;              // *Mob   2.00+     Bit 6: obsolete
    uint16_t setup_move_size = 0;        //  Mob   2.00-2.01
    uint32_t code32_start = 0;           //  Mopre 2.00+
    uint32_t ramdisk_image = 0;          // *Wob   2.00+
    uint32_t ramdisk_size = 0;           // *Wob   2.00+
    uint32_t bootsect_kludge = 0;        //  KI    2.00+     obsolete
    uint16_t heap_end_ptr = 0;           //  Wob   2.01+
    uint8_t  ext_loader_ver = 0;         //  Wop   2.02+
    uint8_t  ext_loader_type = 0;        //  W/Wob 2.02+
    uint32_t cmd_line_ptr = 0;           //  Wob   2.02+
    uint32_t initrd_addr_max = 0;        //  R     2.03+
    uint32_t kernel_alignmnet = 0;       //  RMre  2.05+
    uint8_t  relocatable_kernel = 0;     //  Rre   2.05+
    uint8_t  min_alignment = 0;          //  Rre   2.10+
    uint16_t xloadflags = 0;             //  R     2.12+
    uint32_t cmdline_size = 0;           //  R     2.06+
    uint32_t hardware_subarch = 0;       //  Wop   2.07+
    uint64_t hardware_subarch_data = 0;  //  W     2.07+
    uint32_t paylopad_offset = 0;        //  R     2.08+
    uint32_t payload_length = 0;         //  R     2.08+
    uint64_t setup_data = 0;             //  W     2.09+
    uint64_t pref_address = 0;           //  Rre   2.10+
    uint32_t init_size = 0;              //  R     2.10+
    uint32_t handover_offset = 0;        //  R     2.11+
    uint32_t kernel_info_offset = 0;     //  R     2.15+

    int is_valid();
    int check_setup_sects();
};

#pragma pack(1)
struct e820entry {
    uint64_t addr = 0;
    uint64_t size = 0;
    uint32_t type = 0;
};

#pragma pack(1)
struct boot_params {
    uint8_t      padding0[0x1e8] = { 0 };
    uint8_t      e820_entries = 0;  // 0x1e8
    uint8_t      edd_buf_entries = 0;
    uint8_t      edd_mbr_sig_buf_entries = 0;
    uint8_t      kbd_status = 0;
    uint8_t      padding1[0x5] = { 0 };
    setup_header header;  // 0x1f1
    uint8_t      padding2[0x290-0x1f1-sizeof(setup_header)] = { 0 };
    uint32_t     edd_mbr_sig_buf[BOOT_EDD_MBR_SIG_MAX] = { 0 };  // 0x290
    e820entry    e820map[BOOT_E820_MAP_MAX];  // 0x2d0
    // from arch/x86/include/uapi/asm/bootparam.h L228

 private:
    void increment_e820_entry();

 public:
    void add_e820_entry(uint64_t addr, uint64_t size, uint32_t type);
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
}

#ifdef UNITTEST
void processor_entry_init(
        mpctable_processor_entry entry_array[],
        const int vcpu_num
);
#endif  // UNITTEST

ebda gen_ebda(const int vcpu_num);


#endif  // INCLUDE_BOOT_HPP_
