#include "NES/MemoryNES.hpp"

#include "NES/PPU.hpp"
#include "NES/APU.hpp"

#include <random>
#include <iostream>

MemoryNES::MemoryNES(const std::string &romFilename, APU& apuRef, PPU& ppuRef, Controller& controller1Ref, Controller& controller2Ref)
	: mCartridge(romFilename), mApuRef(apuRef), mPpuRef(ppuRef), mController1Ref(controller1Ref), mController2Ref(controller2Ref)
{
}

void MemoryNES::reset()
{
	// Init random engine
	std::random_device os_seed;
	std::default_random_engine generator(os_seed());
	std::uniform_int_distribution<u16> distribution(0, 255);

	// Set the whole memory to 0
	for (u32 i = 0; i < CPU_RAM_SIZE; i++)
		mCpuRam[i] = (u8)distribution(generator);

	for (u32 i = 0; i < PPU_VRAM_SIZE; i++)
		mPpuVram[i] = (u8)distribution(generator);

	mCartridge.reset();

	mIsOamDmaStarted = false;
	mIsCpuHalt = false;
	mPreviousIsGetCycle = false;
}

u8 MemoryNES::cpuRead(u16 address)
{
	u8 value = 0;
	bool isInCartridgeMemory= mCartridge.readPrg(address, value);
	if (isInCartridgeMemory)
		return value;
	
	if (address < 0x2000)
	{
		// CPU RAM, mirrored 4 times
		value = mCpuRam[address & 0x07FF];
	}
	else if (0x2000 <= address && address < 0x4000)
	{
		// PPU Registers
		value = mPpuRef.readRegister(*this, address & 0x2007);
	}
	else if (0x4000 <= address && address < 0x4018)
	{
		// APU & IO Registers
		if (address == APU_STATUS_CPU_ADDR)
			 // APU status
			value = mApuRef.readRegister(address);

		else if (address == CONTROLLER_1_STATE_ADDR)
			// Controller 1
			value = mController1Ref.getStateBitAndShift();

		else if (address == CONTROLLER_2_STATE_ADDR)
			// Controller 2
			value = mController2Ref.getStateBitAndShift();
	}
	else if (0x4018 <= address && address < 0x4020)
	{
		// APU & IO test registers (unused)
	}

	return value;
}

void MemoryNES::cpuWrite(u16 address, u8 value)
{
	// 0x4020 - 0xFFFF
	bool isInCartridgeMemory= mCartridge.writePrg(address, value);
	if (isInCartridgeMemory)
		return;
	
	if (address < 0x2000)
	{
		// CPU RAM, mirrored 4 times
		mCpuRam[address & 0x07FF] = value;
	}
	else if ((0x2000 <= address && address < 0x4000))
	{
		// PPU Registers
		mPpuRef.writeRegister(*this, address & 0x2007, value);
	}
	else if (0x4000 <= address && address < 0x4018)
	{
		// APU & IO Registers
		if ((0x4000 <= address && address <= 0x4013) || 
		    address == APU_STATUS_CPU_ADDR           || 
			address == APU_FRAME_COUNTER_CPU_ADDR)
		{
			// APU
			mApuRef.writeRegister(address, value);
		}
		else if (address == OAMDMA_CPU_ADDR)
		{
			// OAM DMA
			startOamDma(value);	
		}
		else if (address == CONTROLLER_STROBE_ADDR)
		{
			// Controller 1
			mController1Ref.setStrobe(value);
			mController2Ref.setStrobe(value);
		}
	}
	else if (0x4018 <= address && address < 0x4020)
	{
		// APU & IO test registers (unused)
	}
}

u8 MemoryNES::ppuRead(u16 address, u16 ppuCycleCount)
{
	address &= 0x3FFF;
	u16 mappedNtAddress;

	u8 value = 0;
	bool isInCartridgeMemory = mCartridge.readChr(address, value, mappedNtAddress, ppuCycleCount);
	if (isInCartridgeMemory)
		return value;

	if (0x2000 <= address && address < 0x3F00)
	{
		value = mPpuVram[mappedNtAddress];
	}
	else if (0x3F00 <= address && address < 0x4000)
	{
		value = mPpuRef.readPaletteRam(address);
	}

    return value;
}

void MemoryNES::ppuWrite(u16 address, u8 value, u16 ppuCycleCount)
{
	address &= 0x3FFF;
	u16 mappedNtAddress = 0;
	
	bool isInCartridgeMemory = mCartridge.writeChr(address, value, mappedNtAddress, ppuCycleCount);
	if (isInCartridgeMemory)
		return;

	if (0x2000 <= address && address < 0x3F00)
	{
		mPpuVram[mappedNtAddress] = value;
	}
	else if (0x3F00 <= address && address < 0x4000)
	{
		mPpuRef.writePaletteRam(address, value);
	}
}

s32 MemoryNES::executeOamDma(bool isGetCycle)
{
	s32 elapsedCycles = 1;

	if (!mIsCpuHalt)
	{
		// Do halt cycle
		mIsCpuHalt = true;
	}
	else if (mPreviousIsGetCycle == isGetCycle)
	{
		// Alignement cycle due to OAM & DMC DMA collision
		elapsedCycles = 1;
	}
	else if ((mOamDmaIdx == 0) && !isGetCycle)
	{
		// Do alignment cycle (optionnal)
		elapsedCycles = 1;
	}
	else if ((mOamDmaIdx < 256) && isGetCycle)
	{
		// Read from RAM to DMA
		u16 cpuAddress = (u16)(mOamDma << 8) | mOamDmaIdx;
		mOamDmaBuffer = cpuRead(cpuAddress);

		mOamDmaIdx++;
	}
	else if ((mOamDmaIdx <= 256) && !isGetCycle)
	{
		// Write from DMA to OAM
		cpuWrite(OAMDATA_CPU_ADDR, mOamDmaBuffer);

		if (mOamDmaIdx == 256)
		{
			// DMA Complete
			mIsOamDmaStarted = false;
			mIsCpuHalt = false;
		}
	}
	
	mPreviousIsGetCycle = isGetCycle;

    return elapsedCycles;
}

void MemoryNES::startOamDma(u8 pageAddress)
{
	mOamDma = pageAddress;
	mOamDmaIdx = 0;
	mIsOamDmaStarted = true;
}