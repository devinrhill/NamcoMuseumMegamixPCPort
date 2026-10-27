#include "gotch/DataStorage.hpp"

#include <cstring>

namespace gotch
{
namespace
{
std::uint8_t g_bridgeSave[kBridgeSaveSize];   // sRSOBridge +0x1c
bool g_busy = false;                          // DAT_811dca08

std::uint32_t Get32(const std::uint8_t* p) { return (std::uint32_t)p[0] << 24 | (std::uint32_t)p[1] << 16 | (std::uint32_t)p[2] << 8 | p[3]; }
void Put32(std::uint8_t* p, std::uint32_t v)
{
	p[0] = (std::uint8_t)(v >> 24);
	p[1] = (std::uint8_t)(v >> 16);
	p[2] = (std::uint8_t)(v >> 8);
	p[3] = (std::uint8_t)v;
}
}

std::uint8_t* BridgeSave() { return g_bridgeSave; }
bool GetDataStorageBusy() { return g_busy; }

// RSO2 0x80d022fc + CCDataStorage 0x80d45214 (the blocks come from the module heap, zeroed)
CDataStorage::CDataStorage(std::uint32_t size_) : size(size_ != 0 ? size_ : 0x800)
{
	saved.assign(size, 0);
	work.assign(size, 0);
	transfer.assign(size, 0);
}

// RSO2 0x80d0285c
std::int32_t CDataStorage::GetCheckData(const std::uint8_t* block) const
{
	std::int32_t sum = 0;
	for (std::uint32_t i = 0x30; i < size; i++) sum += block[i];
	return sum;
}

// RSO2 0x80d02424: stamp the work block (0x1f at +magicOffset, the sum at +sumOffset); nothing to do when it equals the saved block, else the
// copy goes out on the next FrameUpdate
std::uint32_t CDataStorage::SaveRequest()
{
	if (request != 0) return state;
	Put32(&work[magicOffset], 0x1f);
	Put32(&work[sumOffset], (std::uint32_t)GetCheckData(work.data()));
	if (std::memcmp(saved.data(), work.data(), size) == 0) return 0;
	request = 1;
	std::memcpy(transfer.data(), work.data(), size);
	wait = 1;
	g_busy = true;
	return CheckComplete();
}

// RSO2 0x80d025b8
std::uint32_t CDataStorage::LoadRequest()
{
	if (request != 0) return state;
	request = 2;
	wait = 1;
	g_busy = true;
	state = 2;
	return state;
}

// RSO2 0x80d02634
std::uint32_t CDataStorage::FrameUpdate()
{
	if (request == 1)
	{
		if (--wait == 0)
		{
			std::memcpy(BridgeSave(), transfer.data(), size < kBridgeSaveSize ? size : kBridgeSaveSize);   // memcpy_s(bridge + 0x1c, ..)
			std::memcpy(saved.data(), transfer.data(), size);
			request = 0;
			state = 0;
			g_busy = false;
		}
	}
	else if (request == 2)
	{
		if (--wait == 0)
		{
			std::memcpy(transfer.data(), BridgeSave(), size < kBridgeSaveSize ? size : kBridgeSaveSize);
			g_busy = false;
			request = 0;
			if (Get32(&transfer[magicOffset]) == 0x1f && Get32(&transfer[sumOffset]) == (std::uint32_t)GetCheckData(transfer.data()))
			{
				std::memcpy(saved.data(), transfer.data(), size);
				state = 0;
				// DAT_80dc9ed8 = 0 (a module flag no ported code reads)
			}
			else
			{
				state = 3;
				valid = false;
				ZeroFillDataBlock();
			}
		}
	}
	else if (request != 0)
		state = 5;
	return state;
}

// RSO2 0x80d02838
void CDataStorage::ActivateLoadedData()
{
	if (state != 0) return;
	std::memcpy(work.data(), saved.data(), size);
}

// RSO2 0x80d02924
std::uint32_t CDataStorage::CheckComplete()
{
	if (request == 2 || request == 1) state = 2;
	else if (request != 0) state = 5;
	return state;
}

// RSO2 0x80d456c0
void CDataStorage::ZeroFillDataBlock()
{
	std::memset(saved.data(), 0, size);
	std::memset(work.data(), 0, size);
	std::memset(transfer.data(), 0, size);
}
}
