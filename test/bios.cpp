#include <gtest/gtest.h>
#include <bios.hpp>

namespace {

TEST(MpGenChecksum, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    ASSERT_EQ(0x5b, mp_gen_checksum(&fps));
}

TEST(MpCalcChecksum, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    fps.checksum = 0x5b;
    ASSERT_EQ(0x00, mp_calc_checksum(&fps));
}

TEST(IsMpChecksumValid, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    fps.checksum = 0x5b;
    ASSERT_TRUE(is_mp_checksum_valid(&fps));
}


}  // namespace
