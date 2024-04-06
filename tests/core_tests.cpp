#include <gtest/gtest.h>
#include <memory>

#include "logger.hpp"
#include "registers.hpp"
#include "machine.hpp"
#include "parser.h"
#include "parser.cpp"

TEST(MachineTests_StartBlock, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartBlock("block1"));
}

TEST(MachineTests_StartBlock, Invalid_InBlock)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_FALSE(machine.TryStartBlock("block2"));
}

TEST(MachineTests_StartBlock, Invalid_ExistingName)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());

	EXPECT_FALSE(machine.TryStartBlock("block1"));
}

TEST(MachineTests_EndBlock, Invalid_NoBlock)
{
	Machine machine;

	EXPECT_FALSE(machine.TryEndBlockMacro());
}

TEST(MachineTests_EndBlock, Invalid_EmptyBlock)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_FALSE(machine.TryEndBlockMacro());
}

TEST(MachineTests_EndBlock, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());
}

TEST(MachineTests_StartMacro, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
}

TEST(MachineTests_StartMacro, Invalid_InMacro)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_FALSE(machine.TryStartMacro("macro2"));
}

TEST(MachineTests_EndMacro, Invalid_NoMacro)
{
	Machine machine;

	EXPECT_FALSE(machine.TryEndBlockMacro());
}

TEST(MachineTests_EndMacro, Invalid_EmptyMacro)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_FALSE(machine.TryEndBlockMacro());
}

TEST(MachineTests_EndMacro, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());
}

TEST(MachineTests_StartMacro, Invalid_SameName)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());

	EXPECT_FALSE(machine.TryStartMacro("macro1"));
}

TEST(MachineTests_SetRegister, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
}

TEST(MachineTests_SetRegister, Invalid_NoBlock)
{
	Machine machine;

	EXPECT_FALSE(machine.TrySetRegister(std::make_unique<FINISH>()));
}

TEST(MachineTests_SetRegister, Invalid_LastRegisterNotFulfilled)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_FALSE(machine.TrySetRegister(std::make_unique<FINISH>()));
}

TEST(MachineTests_SetRegister, Valid_AfterPreviousSet)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_TRUE(machine.TryPushReg(Vec3(0, 0, 0)));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
}

TEST(MachineTests_SetRegister, Invalid_UnexpectedArgument_INT)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_FALSE(machine.TryPushReg(0));
}

TEST(MachineTests_SetRegister, Invalid_UnexpectedArgument_VEC2)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_FALSE(machine.TryPushReg(Vec2(0, 0)));
}

TEST(MachineTests_SetRegister, Invalid_UnexpectedArgument_VEC3)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<TEX0>()));
	EXPECT_FALSE(machine.TryPushReg(Vec3(0, 0, 0)));
}

TEST(MachineTests_SetRegister, Invalid_UnexpectedArgument_VEC4)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_FALSE(machine.TryPushReg(Vec4(0, 0, 0, 0)));
}

TEST(MachineTests_ApplyModifier, Invalid_NoBlock)
{
	Machine machine;

	EXPECT_FALSE(machine.TryApplyModifier(RegModifier::Gouraud));
}

TEST(MachineTests_ApplyModifier, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<PRIM>()));
	EXPECT_TRUE(machine.TryApplyModifier(RegModifier::Gouraud));
}

TEST(MachineTests_ApplyModifier, Invalid_RegisterAcceptsModifiers)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<PRIM>()));
	EXPECT_FALSE(machine.TryApplyModifier(RegModifier::CT32));
}

TEST(MachineTests_ApplyModifier, Invalid_RegisterDoesNotAcceptModifiers)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<XYZ2>()));
	EXPECT_FALSE(machine.TryApplyModifier(RegModifier::CT32));
}

TEST(MachineTests_InsertMacro, Valid)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());
	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_TRUE(machine.TryInsertMacro("macro1"));
}

TEST(MachineTests_InsertMacro, Invalid_DoesNotExist)
{
	Machine machine;

	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());
	EXPECT_TRUE(machine.TryStartBlock("block1"));
	EXPECT_FALSE(machine.TryInsertMacro("macro2"));
}

TEST(MachineTests_InsertMacro, Invalid_NoBlock)
{
	Machine machine;
	EXPECT_TRUE(machine.TryStartMacro("macro1"));
	EXPECT_TRUE(machine.TrySetRegister(std::make_unique<FINISH>()));
	EXPECT_TRUE(machine.TryPushReg(0));
	EXPECT_TRUE(machine.TryEndBlockMacro());
	EXPECT_FALSE(machine.TryInsertMacro("macro1"));
}

int main(void)
{
	logger::g_log_enabled = false;
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}
