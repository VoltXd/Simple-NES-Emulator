#include "CPUTests.hpp"

static void verifyUnmodifiedStatusFlagsFromDEY(const CPU& cpu, const CPU& cpuInitialState)
{
	EXPECT_EQ(cpu.getC(), cpuInitialState.getC());
	EXPECT_EQ(cpu.getI(), cpuInitialState.getI());
	EXPECT_EQ(cpu.getD(), cpuInitialState.getD());
	EXPECT_EQ(cpu.getB(), cpuInitialState.getB());
	EXPECT_EQ(cpu.getV(), cpuInitialState.getV());
}

// ******************** Zero Page ******************** //
// ********** Positive test ********** //
TEST_F(CPUTests, deyWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 yValue = 0x43;
	constexpr u8 targetValue = 0x42;
	constexpr s32 targetCycles = DEY.cycles;

	// Load to A
	memory[TEST_MAIN_ADDRESS] = DEY.opcode;
	cpu.setY(yValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getY(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEY(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Negative test ********** //
TEST_F(CPUTests, deyNegWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 yValue = 0xD7;
	constexpr u8 targetValue = 0xD6;
	constexpr s32 targetCycles = DEY.cycles;

	// Load to A, negative value
	memory[TEST_MAIN_ADDRESS] = DEY.opcode;
	cpu.setY(yValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getY(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_TRUE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEY(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Null test ********** //
TEST_F(CPUTests, deyNullWorks)
{	
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 yValue = 0x01;
	constexpr u8 targetValue = 0x00;
	constexpr s32 targetCycles = DEY.cycles;

	// Load to A, null value
	memory[TEST_MAIN_ADDRESS] = DEY.opcode;
	cpu.setY(yValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getY(), targetValue);
	EXPECT_TRUE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromDEY(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}
