#include <gtest/gtest.h>
#include <kvm.hpp>

namespace {

TEST(KVMStaticFuncTest, GetKVMFD) {
    int r = KVM::getKVMFD();
    ASSERT_GE(3, r);
}

}  // namepspace
