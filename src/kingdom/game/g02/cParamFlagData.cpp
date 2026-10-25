#include "kingdom/game/g02/cParamFlagData.hpp"

namespace kingdom::game::g02
{

// the inline part of DOL 0x80077190: cJetCSV ctor (FUN_801393c0), vtable 0x80391ab4, Init (vtable +0x08), CSV_FLAG -> SetData, +0x20 = 1
cParamFlagData::cParamFlagData()
{
	Init();
	RegistData("CSV_FLAG");
	m_bLoaded = true;
}

// DOL 0x80063214 (Term vtable +0x0c is empty, then the cJetCSV dtor)
cParamFlagData::~cParamFlagData() {}

// DOL 0x80063208
void cParamFlagData::Init() { m_bLoaded = false; }

// the lookup inline in DOL 0x80077190 (cJetCSV::S32(csv, r4 = row, r5 = column 1, 0))
u32 cParamFlagData::nFlag(int nStageInWorld, int nWorld)
{
	const int row = nStageInWorld + (nWorld - 1) * 7;
	if (row < 0 || row >= 0x1c) return 0u;
	return (u32)S32(row, 1, 0);
}

}
