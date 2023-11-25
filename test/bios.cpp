#include <gtest/gtest.h>
#include <bios.hpp>

namespace {

TEST(MpCalcChecksum, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    fps.checksum = 0x5b;
    ASSERT_EQ(0x00, mp_calc_checksum(&fps));
}

TEST(MpGenChecksum, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    ASSERT_EQ(0x5b, mp_gen_checksum(&fps));
}

}  // namespace
