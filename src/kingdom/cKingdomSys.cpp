#include "kingdom/cKingdomSys.hpp"
#include "platform/devtools.hpp"
#include "kingdom/cKingdomSave.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "wii/Archive.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom
{

cKingdomSys::cKingdomSys()
{
	SetName("cKingdomSys");
	m_pSave = new cKingdomSave();
}

cAppKingdom*& cKingdomSys::pAppMain()
{
	return m_pAppMain;
}

// DOL 0x8000d8cc
bool cKingdomSys::LoadGameData(uint block, void* dst, uint size)
{
	return m_pSave != nullptr && m_pSave->ReadBlock(block, dst, size);
}

// DOL 0x8000d8d4
bool cKingdomSys::SaveGameData(uint block, const void* src, uint size)
{
	return m_pSave != nullptr && m_pSave->WriteBlock(block, src, size);
}

bool cKingdomSys::AttachArcData(const string& pArcName, bool visible)
{
	// DOL 0x8000cbb4. The game attaches "<dir>/<name>.lzs" archives from the disc; see wii/Archive.hpp.
	return wii::ArchiveStore::Attach(pArcName);
}

bool cKingdomSys::RegistArcDataAS(const string& pArcName)
{
	return AttachArcData(pArcName);
}

bool cKingdomSys::DetachArcData(const string& pArcName)
{
	return wii::ArchiveStore::Detach(pArcName);
}

sKingdomData* cKingdomSys::SearchDataEx(const string& pKey)
{
	CustomKingdomUtil::Assert(!pKey.empty(), "nullpo");
	if (devtools::G03ExploreHidesData(pKey)) return nullptr; // devtools: --g03-explore hides the stage placement data (not 1:1)
	std::size_t size = 0;
	const std::uint8_t* data = wii::ArchiveStore::Find(pKey, size);
	devtools::G03EditOverride(pKey, data, size); // devtools: the dev window's Pac-N-Roll editor swaps in its edited placement data (not 1:1)
	if (data == nullptr)
	{
		return nullptr;
	}
	// Fresh copy per lookup, owned by the caller (the archive may be detached later).
	sKingdomData* result = new sKingdomData();
	result->nSize = static_cast<uint>(size);
	result->pBin.assign(data, data + size);
	return result;
}

const std::vector<byte>& cKingdomSys::SearchData(const string& pKey)
{
	static const std::vector<byte> empty;
	CustomKingdomUtil::Assert(!pKey.empty(), "nullpo");
	sKingdomData* data = SearchDataEx(pKey);
	return (data != nullptr) ? data->pBin : empty;
}

int cKingdomSys::nMSecToFrame(int nMSec)
{
	return nMSec * 6000 / 100000;
}

}
