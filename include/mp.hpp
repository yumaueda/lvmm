#ifndef MP_H
#define MP_H


constexpr uint8_t  MP_SPEC_REV_1_4          = 4;

// _MP_
constexpr uint32_t MPFPS_INTEL_SIGNATURE    = '_'<<24|'P'<<16|'M'<<8|'_';
constexpr uint8_t  MPFPS_LENGTH             = 1;
constexpr uint8_t  MPFPS_SPEC_REV_1_4       = MP_SPEC_REV_1_4;
constexpr uint8_t  MPFPS_FEAT_CTPRESENT     = 0;
constexpr uint8_t  MPFPS_FEAT_VIRTWIRE      = 0;

// PCMP
constexpr uint32_t MPCTABLE_INTEL_SIGNATURE = 'P'<<24|'M'<<16|'C'<<8|'P';
constexpr uint8_t  MPCTABLE_SPEC_REV_1_4    = MP_SPEC_REV_1_4;
// SUPRMIGR
constexpr uint64_t MPCTABLE_OEM_ID          = 'R'<<56|'G'<<48|'I'<<40|'M'<<32|\


struct mpfps_data = {
    uint32_t signature = MPFPS_INTEL_SIGNATURE;
    uint32_t phys_addr_ptr;
    uint8_t  length = MPFPS_LENGTH;
    uint8_t  spec_rev = MPFPS_SPEC_REV_1_4;
    uint8_t  checksum;
    uint8_t  feat_info_byte_1 = MPFPS_FEAT_CTPRESENT;
    uint8_t  feat_info_byte_2 = MPFPS_FEAT_VIRTWIRE;
    uint8_t  feat_info_byte_3 = 0;
    uint8_t  feat_info_byte_4 = 0;
    uint8_t  feat_info_byte_5 = 0;
};


struct mpctable_header = {
    uint32_t signature = MPCTABLE_INTEL_SIGNATURE;
    uint16_t length;
    uint8_t  spec_rev = MPCTABLE_SPEC_REV_1_4;
    uint8_t  checksum;
    uint64_t oem_id = MPCTABLE_OEM_ID;
    uint8_t  prod_id[12] = { 0 };
    uint32_t oem_table_ptr = 0;
    uint16_t oem_table_size = 0;
    uint16_t entry_count;
    uint32_t addr_local_apic;
    uint16_t extd_table_length;
    uint8_t  extd_table_checksum;
    uint8_t  reserved = 0;
};


class MPFps {
    public:
        explicit MPFps();
        ~MPFps();
        uint8_t calcChecksum();
        mpfps_data data;
};


class MPCtable {
    public:
        explicit MPCTable();
        ~MPCTable();
        mpctable_header header;
}


#endif  // MP_H
