#pragma once

#include "NES/Mapper.hpp"

class Mapper002 : public Mapper
{
public:
	Mapper002(u8 prgNumBanks, u8 chrNumBanks, NametableArrangement ntArr);
	
	void reset() override;
	
	bool mapCpuWrite(u16 address, u32& mappedAddress, u8 value) override;
	bool mapCpuRead(u16 address, u32& mappedAddress) override;
	bool mapPpuWrite(u16 address, u32& mappedAddress) override;
	bool mapPpuRead(u16 address, u32& mappedAddress) override;
	
private:
	bool mapPpuAddress(u16 address, u32& mappedAddress);
	
	u8 mPrgBankIdx;
};
