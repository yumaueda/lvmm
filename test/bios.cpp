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

class MpCtableTest : public testing::Test {
    protected:
        mpctable ctable_default_state;
        mpctable ctable_with_checksum;
        static constexpr int VCPU_NUM = 0x02;
        static constexpr uint8_t VALID_CHECKSUM = 0xb6;  // NOTE: we haven't verified that 0xb6 is the really valid value.

        virtual void SetUp() {
            processor_entry_init(ctable_default_state.processor_entry, VCPU_NUM);
            ctable_with_checksum = ctable_default_state;
            ctable_with_checksum.checksum = mp_gen_checksum(&ctable_with_checksum);
        }
};


// MpFpsTest
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

// MpCtableTest
TEST_F(MpCtableTest, CalcChecksum) {
    ASSERT_TRUE(mp_calc_checksum(&ctable_default_state));
    ASSERT_FALSE(mp_calc_checksum(&ctable_with_checksum));
}

TEST_F(MpCtableTest, GenChecksum) {
    ASSERT_EQ(VALID_CHECKSUM, mp_gen_checksum(&ctable_default_state));
}

TEST_F(MpCtableTest, IsMpChecksumValid) {
    ASSERT_FALSE(is_mp_checksum_valid(&ctable_default_state));
    ASSERT_TRUE(is_mp_checksum_valid(&ctable_with_checksum));
}

}  // namespace
