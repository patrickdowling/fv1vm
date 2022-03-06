#include <gtest/gtest.h>

#include "vm/engines/engine_i32_v1.h"
#include "vm/vm_types.h"

namespace fv1tests {

using namespace fv1;
using namespace fv1::engine;

using Register = EngineI32::Register;
using Constant = EngineI32::Constant;

TEST(TestVMTypes, Register)
{
  Register reg;
  EXPECT_EQ(0, reg.loadi());
}

TEST(TestVMTypes, RegisterCmp)
{
  Register reg;
  EXPECT_FALSE(reg.neg());
  EXPECT_TRUE(reg.zero());
  EXPECT_TRUE(reg.gez());

  reg.store(1);
  EXPECT_FALSE(reg.neg());
  EXPECT_FALSE(reg.zero());
  EXPECT_TRUE(reg.gez());

  reg.store(-1);
  EXPECT_TRUE(reg.neg());
  EXPECT_FALSE(reg.zero());
  EXPECT_FALSE(reg.gez());
}

TEST(TestVMTypes, Constant)
{
  Constant constant;
  EXPECT_EQ(0, constant.loadi());
}

}  // namespace fv1tests
