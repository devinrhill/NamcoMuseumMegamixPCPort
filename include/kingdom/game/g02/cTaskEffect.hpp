#pragma once

#include "common.hpp"
#include "kingdom/cTaskPBPtkMan.hpp"

// Rally-X Remix (g02) particle manager: kingdom::game::g02::cTaskEffect (0xd50 bytes, vtable 0x80391b8c, parent kingdom::cTaskPBPtkMan; the
// cJetPtkMan sub-object at +0x1c0 has its own vtable 0x80391be4). Built by the app ctor 0x801236bc: new 0xd50 + ctor 0x801245e4 (cSysTaskUser
// priority 0x10000, the cTaskPushBack / cTaskPBPtkMan chain, then setup 0x8006cb10), stored as the app's particle manager (0x800a57a8 = app
// +0x1bc, natively cAppKingdom::pTaskPtkMan()) and connected to the app. Every cKingdomPtkHnd::Generate of the game (the cars' handles in
// cTaskUnit, the flag / item effects) lands in it through pAppMain()->pTaskPtkMan(), so the cars need no pointer to it.
// Setup 0x8006cb10: Create(0x2c slots, 0x100 emitters, 0x200 generators, 0x2000 particles) (0x80021b80), slot i = the PTK entry below
// (RegistData by CRC 0x80021c58; natively by entry name, the port's CRC overload is empty); slot 0x28 only when the stage archive has
// PTK_STAGE_EFF01 (0x8000bca0).
namespace kingdom::game::g02
{
class cRallyUnitHost;

class cTaskEffect : public kingdom::cTaskPBPtkMan
{
public:
	enum ESlot
	{
		eTEST = 0x00, eSMOKE = 0x01, eNITRO = 0x02, eCRASH = 0x03, eCHIP = 0x04, eOIL = 0x05, eMUD = 0x06, eSPARK = 0x07, eARMOR = 0x08,
		eGETS = 0x09, eEXHAUST = 0x0a, eEFFECT_LINE = 0x0b, eSMOKE2 = 0x0c, eSMOKE3 = 0x0d, eSMOKE4 = 0x0e, eCHAMELEON = 0x0f, eSAND = 0x10,
		eBANG = 0x11, eSCORE_MULT2 = 0x12, eSCORE01 = 0x13,   // .. eSCORE01 + 19 = PTK_SCORE20 (0x26)
		eSTONE_BRAEK = 0x27, eSTAGE_EFF01 = 0x28, eQUESTION = 0x29, eFLAG = 0x2a, eENPTY = 0x2b,
		eSLOT_MAX = 0x2c
	};
	static const char* const SLOT_NAME[eSLOT_MAX];

	explicit cTaskEffect(cRallyUnitHost* pHost);           // DOL 0x801245e4 + setup 0x8006cb10 (pHost = the app: the DOL's parent +0x14)
	bool Execute(cJetTask* pParam) override;               // DOL 0x8006cedc

	// DOL 0x8006cfa0 (caller cTaskUnit 0x800881dc, single player only): the one-time score effect PTK_SCORE01..20 of the flag count
	// (table 0x8036c2a8: count 0 and 1 -> PTK_SCORE01, n -> PTK_SCORE<n>, up to 20) and with bMult2 also PTK_SCORE_MULT2, at pos
	void FlagScore(int nCount, bool bMult2, const Vector4& pos);
	void RedFlag(const Vector4& pos);                      // DOL 0x8006d114 (caller red car 0x8006efc0): one-time PTK_FLAG at pos
	void Clear();                                          // FUN_8008a814 (cJetPtkMan at +0x1c0; caller app reset 0x80125a14): every effect off

private:
	cRallyUnitHost* m_pHost{};                             // the DOL's parent (+0x14), dynamic_cast to cAppG02RallyXGame
};
}
