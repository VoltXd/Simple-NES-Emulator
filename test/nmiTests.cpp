#include "CPUTests.hpp"

TEST_F(CPUTests, entersNmi)
{
	// Target values
	constexpr u16 targetAddress = 0x425D;
	constexpr u8 targetLsb = 0x5D;
	constexpr u8 targetMsb = 0x42;
	constexpr s32 targetCycles = BRK.cycles;

	// Run program
	cpu.setI(0);
	cpu.setC(1);
	cpu.setV(1);
	cpu.setN(1);
	memory[NMI_VECTOR_LSB] = targetLsb;
	memory[NMI_VECTOR_MSB] = targetMsb;
	s32 elapsedCycles = cpu.nmi(memory);

	// Verify
	EXPECT_EQ(cpu.getSp(), 0xFF - 5);
	EXPECT_EQ(cpu.getPc(), targetAddress);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFB], 0b1110'0001);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFC], (TEST_MAIN_ADDRESS) & 0x00FF);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFD], ((TEST_MAIN_ADDRESS) & 0xFF00) >> 8);
	EXPECT_EQ(elapsedCycles, targetCycles);
}

TEST_F(CPUTests, EntersNmiAnyway)
{
	// Target values
	constexpr u16 targetAddress = 0x425D;
	constexpr u8 targetLsb = 0x5D;
	constexpr u8 targetMsb = 0x42;
	constexpr s32 targetCycles = 7;

	// Run program
	cpu.setI(1);
	cpu.setC(1);
	cpu.setV(1);
	cpu.setN(1);
	memory[NMI_VECTOR_LSB] = targetLsb;
	memory[NMI_VECTOR_MSB] = targetMsb;
	s32 elapsedCycles = cpu.nmi(memory);

	// Verify
	EXPECT_EQ(cpu.getSp(), 0xFF - 5);
	EXPECT_EQ(cpu.getPc(), targetAddress);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFB], 0b1110'0101);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFC], (TEST_MAIN_ADDRESS) & 0x00FF);
	EXPECT_EQ(memory[SP_PAGE_OFFSET | 0xFD], ((TEST_MAIN_ADDRESS) & 0xFF00) >> 8);
	EXPECT_EQ(elapsedCycles, targetCycles);
}