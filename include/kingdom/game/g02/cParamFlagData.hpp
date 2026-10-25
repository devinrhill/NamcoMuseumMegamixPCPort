#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

// Rally-X Remix (g02) flag parameters: kingdom::game::g02::cParamFlagData (a cJetCSV with its vtable at +0x1c: 0x80391ab4; slot +0x08 Init
// 0x80063208, +0x0c Term (empty, 0x800632a8), +0x10 dtor 0x80063214). The DOL has no ctor of its own: cTaskMap 0x80077190 builds one on the
// stack (cJetCSV ctor FUN_801393c0, vtable store, Init), loads CSV_FLAG (0x76bc838f, data lookup FUN_8000bcd4 -> SetData FUN_80139428), sets
// +0x20 = 1 and reads column 1 of row (stage in world + (world - 1) * 7) = how many of the stage's type 6 flag candidates get a flag.
namespace kingdom::game::g02
{
class cParamFlagData : public kingdom::cKingdomCSV
{
public:
	cParamFlagData();                  // the inline part of DOL 0x80077190 (ctor + Init + SetData + "+0x20 = 1")
	~cParamFlagData() override;        // DOL 0x80063214

	void Init();                       // DOL 0x80063208: +0x20 = 0
	// the lookup inline in DOL 0x80077190: row = nStageInWorld + (nWorld - 1) * 7; rows < 0 or >= 0x1c give 0, else S32(row, 1, 0)
	u32 nFlag(int nStageInWorld, int nWorld);

	bool m_bLoaded{};                  // +0x20 (set after SetData whether or not the data was found, like the DOL)
};
}
