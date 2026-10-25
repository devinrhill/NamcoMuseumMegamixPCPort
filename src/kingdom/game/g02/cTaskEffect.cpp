#include "kingdom/game/g02/cTaskEffect.hpp"

#include <cstddef>

#include "GamePad.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/cKingdomPtkMan.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"
#include "wii/Archive.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float FRAME_STEP = 1.0f;   // FLOAT_8047a950
constexpr float NO_DELAY = 0.0f;     // FLOAT_8047a954

// table 0x8036c2a8 (21 words, copied to the stack by 0x8006cfa0): the score slot by flag count
constexpr uint SCORE_SLOT[21] = {0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
                                 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26};

// a one-time effect through a local handle (the DOL's stack cKingdomPtkHnd, Init PTR_80393f60 / dtor DAT_80393f64): FUN_80021a9c(f1 = 0
// delay, hnd, slot, onetime 1, 0, 0, pos, rotate 0, scale 0, light 0, fog -1)
void OneTime(uint nSlot, const Vector4& pos)
{
	kingdom::cKingdomPtkHnd h;
	h.Generate(nSlot, true, false, false, NO_DELAY, pos, nullptr, nullptr, 0, -1);
}
}

// DOL 0x8006cb10 (CRCs of these names; slot 0x0c = 0x04529f8a, PTK_SMOKE2 in g02game.lzs)
const char* const cTaskEffect::SLOT_NAME[eSLOT_MAX] = {
	"PTK_TEST", "PTK_SMOKE", "PTK_NITRO", "PTK_CRASH", "PTK_CHIP", "PTK_OIL", "PTK_MUD", "PTK_SPARK", "PTK_ARMOR", "PTK_GETS",
	"PTK_EXHAUST", "PTK_EFFECT_LINE", "PTK_SMOKE2", "PTK_SMOKE3", "PTK_SMOKE4", "PTK_CHAMELEON", "PTK_SAND", "PTK_BANG", "PTK_SCORE_MULT2",
	"PTK_SCORE01", "PTK_SCORE02", "PTK_SCORE03", "PTK_SCORE04", "PTK_SCORE05", "PTK_SCORE06", "PTK_SCORE07", "PTK_SCORE08", "PTK_SCORE09",
	"PTK_SCORE10", "PTK_SCORE11", "PTK_SCORE12", "PTK_SCORE13", "PTK_SCORE14", "PTK_SCORE15", "PTK_SCORE16", "PTK_SCORE17", "PTK_SCORE18",
	"PTK_SCORE19", "PTK_SCORE20", "PTK_STONE_BRAEK", "PTK_STAGE_EFF01", "PTK_QUESTION", "PTK_FLAG", "PTK_ENPTY"};

// DOL 0x801245e4 (+ setup 0x8006cb10). The cSysTaskUser priority 0x10000 of the ctor is not set natively (the other games' effect ports do
// not set one either).
cTaskEffect::cTaskEffect(cRallyUnitHost* pHost) : m_pHost(pHost)
{
	SetName("g02::cTaskEffect");
	kingdom::cKingdomPtkMan* man = get_KingdomPtkMan();
	man->Create(eSLOT_MAX, 0x100u, 0x200u, 0x2000u);   // 0x80021b80
	for (uint i = 0; i < eSLOT_MAX; i++)
	{
		if (i == eSTAGE_EFF01)
		{
			std::size_t size = 0;   // 0x8000bca0: only when the stage archive has it
			if (wii::ArchiveStore::Find(SLOT_NAME[i], size) == nullptr) continue;
		}
		man->RegistData(i, SLOT_NAME[i]);   // 0x80021c58
	}
}

// DOL 0x8006cedc: the particle calc (FUN_8008ad34(1.0, +0x1c0)) unless the app is paused (+0x1b4), the stage end runs (+0x284) or HOME is
// held (get_pad1(0) & 0x400, tested twice in the DOL)
bool cTaskEffect::Execute(cJetTask* pParam)
{
	(void)pParam;
	const bool home = (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
	const bool frozen = (m_pHost != nullptr && (m_pHost->IsUnitPause() || m_pHost->IsUnitStageEnd())) || home;
	if (!frozen) get_KingdomPtkMan()->UpdateFrame(FRAME_STEP);
	return true;
}

// DOL 0x8006cfa0 (the DOL indexes its 21-entry table unchecked; natively clamped to 0..20)
void cTaskEffect::FlagScore(int nCount, bool bMult2, const Vector4& pos)
{
	const int i = nCount < 0 ? 0 : (nCount > 20 ? 20 : nCount);
	OneTime(SCORE_SLOT[i], pos);
	if (bMult2) OneTime(eSCORE_MULT2, pos);
}

// DOL 0x8006d114
void cTaskEffect::RedFlag(const Vector4& pos) { OneTime(eFLAG, pos); }

// FUN_8008a814 (the cJetPtkMan sub-object: every emitter detached and freed, every generator / particle freed). UNVERIFIED that it is the
// C#'s cJetPtkMan::Genocide (same effect: the emitter / generator / particle lists emptied).
void cTaskEffect::Clear() { get_KingdomPtkMan()->Genocide(); }

}
