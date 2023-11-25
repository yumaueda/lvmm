#include <gtest/gtest.h>
#include <bios.hpp>

namespace {

class MpFpsTest : public testing::Test {
    protected:
        mpfps fps_default_state;
        mpfps fps_with_checksum;
        static constexpr uint8_t VALID_CHECKSUM = 0x5b;

        virtual void SetUp() {
            fps_default_state.phys_addr_ptr = EBDA_START+0x40;
            fps_with_checksum.checksum = mp_gen_checksum(&fps_with_checksum);
        }
};

/*
class MpCtableTest : public testing::Test {
    protected:
        mpctable
};
*/

TEST_F(MpFpsTest, CalcChecksum) {
    ASSERT_TRUE(mp_calc_checksum(&fps_default_state));
    ASSERT_FALSE(mp_calc_checksum(&fps_with_checksum));
}

TEST_F(MpFpsTest, GenChecksum) {
    ASSERT_EQ(VALID_CHECKSUM, mp_gen_checksum(&fps_default_state));
}

TEST_F(MpFpsTest, IsMpChecksumValid) {
    ASSERT_FALSE(is_mp_checksum_valid(&fps_default_state));
    ASSERT_TRUE(is_mp_checksum_valid(&fps_with_checksum));
}

}  // namespace
