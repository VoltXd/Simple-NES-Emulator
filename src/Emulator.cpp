#include "Emulator.hpp"

#include <cstdlib>
#include <iostream>
#include "NES/Cartridge.hpp"
#include "IO/GlfwRenderer.hpp"

Emulator::Emulator(const std::string &romFilename)
	: mMemory(romFilename, mPpu)
{
}

int Emulator::run()
{
	// Instantiations
	
	// *************** CPU Emulation *************** //
	// Reset
	mMemory.reset();
	mCpu.reset(mMemory);
	mPpu.reset();

	GlfwRenderer renderer;
	// picture_t redScreen = { 0 }; 
	// for (int i = 0; i < PPU_OUTPUT_HEIGHT; i++)
	// 	for (int j = 0; j < PPU_OUTPUT_WIDTH; j++)
	// 	{
	// 		redScreen[i][j][0] = (u8)i;
	// 		redScreen[i][j][1] = (u8)j;
	// 		redScreen[i][j][2] = (u8)sqrt(i * j);
	// 	}
	
		
	// Infinite loop
	while (true)
	{
		runOneInstruction();
		if (mPpu.isImageReady())
		{
			mPpu.clearIsImageReady();
			renderer.draw(mPpu.getPicture());
		}
	}

	return EXIT_SUCCESS;
}

void Emulator::runOneInstruction()
{
	// u16 pc = mCpu.getPc();
	// u8 instruction = mMemory.cpuRead(pc);

	s32 elapsedCycles;
	if(mPpu.getVBlankNMISignal())
	{
		elapsedCycles = mCpu.nmi(mMemory);
		mPpu.clearNMISignal();	
	}
	else
		elapsedCycles = mCpu.execute(1, mMemory);
	
	for (int i = 0; i < 3 * elapsedCycles; i++)
		mPpu.executeOneCycle(mMemory);

	// printCpuInfo(pc, instruction, elapsedCycles);
}

void Emulator::printCpuInfo(u16 pc, u8 instruction, s32 elapsedCycles)
{
	std::cout << std::uppercase << std::hex 
	          << "PC: 0x" << pc
	          << ", INST: 0x" << +instruction
			  << std::dec << ", cycles: " << +elapsedCycles 
			  << std::endl;
}
