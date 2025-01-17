#include "CPUTests.hpp"

static void verifyUnmodifiedStatusFlagsFromDEX(const CPU& cpu, const CPU& cpuInitialState)
{
	EXPECT_EQ(cpu.getC(), cpuInitialState.getC());
	EXPECT_EQ(cpu.getI(), cpuInitialState.getI());
	EXPECT_EQ(cpu.getD(), cpuInitialState.getD());
	EXPECT_EQ(cpu.getB(), cpuInitialState.getB());
	EXPECT_EQ(cpu.getV(), cpuInitialState.getV());
}

// ******************** Zero Page ******************** //
// ********** Positive test ********** //
TEST_F(CPUTests, dexWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 xValue = 0x43;
	constexpr u8 targetValue = 0x42;
	constexpr s32 targetCycles = DEX.cycles;

	// Load to A
	memory[TEST_MAIN_ADDRESS] = DEX.opcode;
	cpu.setX(xValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Negative test ********** //
TEST_F(CPUTests, dexNegWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 xValue = 0xD7;
	constexpr u8 targetValue = 0xD6;
	constexpr s32 targetCycles = DEX.cycles;

	// Load to A, negative value
	memory[TEST_MAIN_ADDRESS] = DEX.opcode;
	cpu.setX(xValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_TRUE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Null test ********** //
TEST_F(CPUTests, dexNullWorks)
{	
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 xValue = 0x01;
	constexpr u8 targetValue = 0x00;
	constexpr s32 targetCycles = DEX.cycles;

	// Load to A, null value
	memory[TEST_MAIN_ADDRESS] = DEX.opcode;
	cpu.setX(xValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_TRUE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}
