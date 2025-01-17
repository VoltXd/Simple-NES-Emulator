#include "CPUTests.hpp"

static void verifyUnmodifiedStatusFlagsFromTAX(const CPU& cpu, const CPU& cpuInitialState)
{
	EXPECT_EQ(cpu.getC(), cpuInitialState.getC());
	EXPECT_EQ(cpu.getI(), cpuInitialState.getI());
	EXPECT_EQ(cpu.getD(), cpuInitialState.getD());
	EXPECT_EQ(cpu.getB(), cpuInitialState.getB());
	EXPECT_EQ(cpu.getV(), cpuInitialState.getV());
}

// ********** Positive test ********** //
TEST_F(CPUTests, taxPosWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 targetValue = 0x42;
	constexpr s32 targetCycles = TAX.cycles;

	// Transfert A to X
	memory[TEST_MAIN_ADDRESS] = TAX.opcode;
	cpu.setA(targetValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromTAX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Negative test ********** //
TEST_F(CPUTests, taxNegWorks)
{
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 targetValue = 0xD8;
	constexpr s32 targetCycles = TAX.cycles;

	// Transfert A to X
	memory[TEST_MAIN_ADDRESS] = TAX.opcode;
	cpu.setA(targetValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_FALSE(cpu.getZ());
	EXPECT_TRUE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromTAX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

// ********** Null test ********** //
TEST_F(CPUTests, taxNullWorks)
{	
	// Target values
	const CPU cpuInitialState = cpu;
	constexpr u8 targetValue = 0x00;
	constexpr s32 targetCycles = TAX.cycles;

	// Transfert A to X
	memory[TEST_MAIN_ADDRESS] = TAX.opcode;
	cpu.setA(targetValue);
	s32 elapsedCycles = cpu.execute(targetCycles, memory);

	// Verify
	EXPECT_EQ(cpu.getX(), targetValue);
	EXPECT_TRUE(cpu.getZ());
	EXPECT_FALSE(cpu.getN());
	verifyUnmodifiedStatusFlagsFromTAX(cpu, cpuInitialState);
	EXPECT_EQ(elapsedCycles, targetCycles);
}
