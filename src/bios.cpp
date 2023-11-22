#include <iostream>//debug

#include <bios.hpp>


template <>
uint8_t mp_calc_checksum<mpfps*>(mpfps* mpptr) {
    uint8_t checksum = 0;
    uint8_t* byteptr_fps = reinterpret_cast<uint8_t*>(mpptr);

    for (int i = 0; i < MPFPS_LENGTH*PARAGRAPH_SIZE; ++i)
        checksum += *byteptr_fps++;

    return checksum;
}

template <>
uint8_t mp_calc_checksum<mpctable*>(mpctable* mpptr) {
    uint8_t checksum = 0;
    uint8_t* byteptr_ctable = reinterpret_cast<uint8_t*>(mpptr);

    for (int i = 0; i < MPCTABLE_LENGTH; ++i)
        checksum += *byteptr_ctable++;

    return checksum;
}


ebda gen_ebda(const int vcpu_num) {
    std::cout << std::string(__func__) << " called" << std::endl;
    ebda ebda_ret;

    ebda_ret.fps.phys_addr_ptr = EBDA_START + 0x40;
    ebda_ret.fps.checksum = mp_gen_checksum(&ebda_ret.fps);

    for (uint8_t i=0; i<vcpu_num; ++i) {
        ebda_ret.ctable.processor_entry[i].cpu_flags |= MPCTE_PROC_CPUFLAGS_EN;
        ebda_ret.ctable.processor_entry[i].cpu_flags |= i==0 ? MPCTE_PROC_CPUFLAGS_BP : 0;
        ebda_ret.ctable.processor_entry[i].local_apic_id = i;
        //ebda_ret.ctable.processor_entry[i].cpu_sig 
        //ebda_ret.ctable.processor_entry[i].feat_flags
    }

    ebda_ret.ctable.checksum = mp_gen_checksum(&ebda_ret.ctable);

    return ebda_ret;
}
