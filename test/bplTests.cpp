#include "CPUTests.hpp"

// ************** Cross page ************** //
TEST_F(CPUTests, bplCrossesPage)
{
	// Target values
	constexpr u8 relAddress = 255 - 15;
	constexpr u16 targetPc = TEST_MAIN_ADDRESS - 14;

	constexpr s32 targetCycles = BPL.cycles;

	// Run program
	memory[TEST_MAIN_ADDRESS] = BPL.opcode;
	memory[TEST_MAIN_ADDRESS + 1] = relAddress;
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getPc(), targetPc);
	EXPECT_EQ(elapsedCycles, targetCycles + 2);
}

// ************** Not Branch ************** //
TEST_F(CPUTests, bplDoesntBranch)
{
	// Target values
	constexpr u8 relAddress = 255 - 15;
	constexpr u16 targetPc = TEST_MAIN_ADDRESS + 2;

	constexpr s32 targetCycles = BPL.cycles;

	// Run program
	cpu.setN(1);
	memory[TEST_MAIN_ADDRESS] = BPL.opcode;
	memory[TEST_MAIN_ADDRESS + 1] = relAddress;
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getPc(), targetPc);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ************** Branches ************** //
TEST_F(CPUTests, bplBranches)
{
	// Target values
	constexpr u8 relAddress = 0x08;
	constexpr u16 targetPc = 0x800A;

	constexpr s32 targetCycles = BPL.cycles;

	// Run program
	memory[TEST_MAIN_ADDRESS] = BPL.opcode;
	memory[TEST_MAIN_ADDRESS + 1] = relAddress;
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getPc(), targetPc);
	EXPECT_EQ(elapsedCycles, targetCycles + 1); 
}