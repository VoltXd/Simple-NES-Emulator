#include "Emulator.hpp"

#include <cstdlib>
#include <iostream>
#include <chrono>
#include "NES/Cartridge.hpp"
#include "IO/GlfwRenderer.hpp"

Emulator::Emulator(const std::string &romFilename)
	: mMemory(romFilename, mPpu)
{
}

int Emulator::run()
{
	// *************** NES Emulation *************** //
	// Reset
	mMemory.reset();
	mCpu.reset(mMemory);
	mPpu.reset();
	mIsDmaGetCycle = false;

	GlfwRenderer renderer;

	std::chrono::steady_clock::time_point time0 = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point time1;
	std::chrono::duration<double> elapsedTime;
		
	// Infinite loop
	while (true)
	{
		// Update
		runOneInstruction();
		if (mPpu.isImageReady())
		{
			mPpu.clearIsImageReady();

			// Wait before rendering -> 60 FPS
			do
			{
				time1 = std::chrono::steady_clock::now();
				elapsedTime = time1 - time0;

			} while (elapsedTime.count() < (1.0 / 60.0988));
			time0 = time1;
			
			// Render
			renderer.draw(mPpu.getPicture());
		}
	}

	return EXIT_SUCCESS;
}

void Emulator::runOneInstruction()
{
	// u16 pc = mCpu.getPc();
	// u8 instruction = mMemory.cpuRead(pc);

	// CPU
	s32 elapsedCycles;
	if (mMemory.isOamDmaStarted())
		elapsedCycles = mMemory.executeOamDma(mIsDmaGetCycle);
	else if(mPpu.getVBlankNMISignal())
	{
		elapsedCycles = mCpu.nmi(mMemory);
		mPpu.clearNMISignal();	
	}
	else
		elapsedCycles = mCpu.execute(1, mMemory);

	// Update get/put cycle flag (DMA)
	mIsDmaGetCycle = (bool)(((mIsDmaGetCycle ? 1 : 0) + elapsedCycles) % 2);
	
	// PPU
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
