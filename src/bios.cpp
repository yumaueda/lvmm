#include <bios.hpp>


static inline
uint8_t _mp_calc_checksum_inline(uint8_t *byteptr_mp, int length) {
    uint8_t checksum = 0;
    for (int i = 0; i < length; ++i)
        checksum += *byteptr_mp++;
    return checksum;
}

template <>
uint8_t mp_calc_checksum<mpfps*>(mpfps* mpptr) {
    uint8_t* byteptr_fps = reinterpret_cast<uint8_t*>(mpptr);
    return _mp_calc_checksum_inline(byteptr_fps, MPFPS_LENGTH*PARAGRAPH_SIZE);
}

template <>
uint8_t mp_calc_checksum<mpctable*>(mpctable* mpptr) {
    uint8_t* byteptr_ctable = reinterpret_cast<uint8_t*>(mpptr);
    return _mp_calc_checksum_inline(byteptr_ctable, MPCTABLE_LENGTH);
}

ebda gen_ebda(const int vcpu_num) {
    ebda ebda_ret;

    // checksum is initialized to zero. check bios.hpp

    // see definition of struct ebda
    ebda_ret.fps.phys_addr_ptr = EBDA_START + 0x40;
    ebda_ret.fps.checksum = mp_gen_checksum(&ebda_ret.fps);

    for (uint8_t i=0; i<vcpu_num; ++i) {
        ebda_ret.ctable.processor_entry[i].local_apic_id = i;
        ebda_ret.ctable.processor_entry[i].cpu_flags |= MPCTE_PROC_CPUFLAGS_EN;
        ebda_ret.ctable.processor_entry[i].cpu_flags |= i==0 ? \
            MPCTE_PROC_CPUFLAGS_BP : 0;
        ebda_ret.ctable.processor_entry[i].cpu_sig = MPCTE_PROC_CPUSIGNATURE;
    }

    ebda_ret.ctable.checksum = mp_gen_checksum(&ebda_ret.ctable);

    return ebda_ret;
}
