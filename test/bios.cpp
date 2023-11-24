#include <gtest/gtest.h>
#include <bios.hpp>

namespace {

TEST(MpGenChecksum, HandleMpFps) {
    mpfps fps;
    fps.phys_addr_ptr = EBDA_START+0x40;
    // Haven't made sure whether 0x5b is a really valid value
    ASSERT_EQ(0x5b, mp_gen_checksum(&fps));
}

}  // namespace
