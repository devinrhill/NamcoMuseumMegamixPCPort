#include <cstdio>
#include "kingdom/game/g02/cTaskGimmick.hpp"

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cTaskItem.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float ROT_DIR1 = 3.1415927f;   // FLOAT_8047af90
constexpr float ROT_DIR2 = 4.712389f;    // FLOAT_8047af94
constexpr float ROT_DIR3 = 1.5707964f;   // FLOAT_8047af98

bool PtkReady()
{
	kingdom::cKingdomSys* sys = KingdomGlobal::KingSys;
	return sys != nullptr && sys->pAppMain() != nullptr && sys->pAppMain()->pTaskPtkMan() != nullptr;
}

cRallyUnitHost* Host()
{
	kingdom::cKingdomSys* sys = KingdomGlobal::KingSys;
	return sys != nullptr ? dynamic_cast<cRallyUnitHost*>(sys->pAppMain()) : nullptr;
}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// cParamGimmick

int cParamGimmick::s_nRef = 0;
cParamGimmick* cParamGimmick::s_pParam = nullptr;

// the cParamGimmick part of FUN_800b1944: cJetCSV ctor FUN_801393c0, CSV_GIMMICK (FUN_8000bcd4 -> FUN_80139428), +0x20 = row 0 column 1, +0x24 = 1
cParamGimmick::cParamGimmick()
{
	RegistData("CSV_GIMMICK");
	fValue = F32(0, 1, 0.0f);
	bLoaded = true;
}

// DOL 0x800b1944 (the reference count part)
void cParamGimmick::AddRef()
{
	if (s_nRef++ == 0) s_pParam = new cParamGimmick();
}

// DOL 0x800b1a48
void cParamGimmick::Release()
{
	if (--s_nRef == 0 && s_pParam != nullptr)
	{
		delete s_pParam;   // vtable +0x10 (dtor FUN_800b1aa0, delete)
		s_pParam = nullptr;
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// cTaskGimmick

cTaskGimmick::cTaskGimmick()
{
	SetName("g02::cTaskGimmick");
	m_nX = 0;
	m_nZ = 0;
	InitGimmick();   // vtable +0x3c while the cTaskGimmick vtable is installed
}

// DOL 0x80070830
cTaskGimmick::~cTaskGimmick() {}

// DOL 0x800b1944
void cTaskGimmick::InitGimmick()
{
	m_nType = -1;
	m_nDir = 0;
	m_bActive = true;
	cParamGimmick::AddRef();
}

// DOL 0x800b1a48
void cTaskGimmick::Term()
{
	cParamGimmick::Release();
	cTaskModel::Term();
}

// DOL 0x800b1b38
bool cTaskGimmick::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (IsFrozen()) return true;
	kingdom::cKingdomModel* m = pModel(0);
	if (m == nullptr) return true;
	m->vTrans() = Vector4((float)(m_nX - 4), 0.0f, (float)(m_nZ - 8), 1.0f);
	Vector4& rot = m->vRotate();
	RotY(rot);
	m->UpdateFrame();   // model vtable +0x50 slot +0x14
	return true;
}

// DOL 0x800b1d24
bool cTaskGimmick::IsAt(int x, int z) const { return m_nX == x && m_nZ == z; }

bool cTaskGimmick::IsFrozen()
{
	const bool home = (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
	const cRallyUnitHost* h = Host();
	return home || (h != nullptr && (h->IsUnitPause() || h->IsUnitStageEnd()));
}

// DOL 0x800b1d74 (dir 3: 0x800b1dc4)
void cTaskGimmick::RotY(Vector4& rot) const
{
	switch (m_nDir)
	{
	case 1: rot.y = ROT_DIR1; break;
	case 2: rot.y = ROT_DIR2; break;
	case 3: rot.y = ROT_DIR3; break;
	default: rot.y = 0.0f; break;   // FLOAT_8047af88 (0 and anything outside 0..3)
	}
}

// DOL 0x800b1dd0
bool cTaskGimmick::IsCollector(cTaskUnit* pUnit)
{
	if (pUnit == nullptr) return false;
	const cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	const bool boss = sys != nullptr && sys->IsBossStage();
	return (pUnit->nUnitId() & (boss ? 0x20u : 0x10u)) != 0;   // vtable +0x44
}

// DOL 0x800c5ce0 (the same pick as cRallyModelShadow): table 0x80371188 {WORLD1, WORLD1, WORLD2, WORLD3, WORLD4} by world; multiplayer by
// the stage in the world (the DOL reads past the table for 5 / 6)
const char* cTaskGimmick::ShadowClip()
{
	static const char* const CLIP[5] = {"SHADOW_COLOR_WORLD1", "SHADOW_COLOR_WORLD1", "SHADOW_COLOR_WORLD2", "SHADOW_COLOR_WORLD3",
	                                    "SHADOW_COLOR_WORLD4"};
	const cG02RallyXSys* sys = KingdomGlobal::RallyXSys;
	int i = 0;
	if (sys != nullptr) i = sys->IsMulti() ? (sys->nStageIdx() == 0 ? 0 : (sys->nStageIdx() - 1) % 6 + 1) : sys->nWorld();
	return i >= 0 && i < 5 ? CLIP[i] : CLIP[0];
}

kingdom::cKingdomModel* cTaskGimmick::SetupModel(uint nIdx, const char* key, const char* clip, bool bShadow)
{
	kingdom::cKingdomModel* m = CreateModel(nIdx);   // FUN_8009991c
	m->RegistModel(key, 0x36f);                       // FUN_8001f72c
	m->SetLightSetIdx(1);                             // FUN_8001e1f8
	m->SetFogIdx(1);                                  // FUN_8001e400
	if (clip != nullptr) m->RegistAnm(0u, clip);      // FUN_8001a4d8(m, 0, crc, 0, 1)
	if (bShadow)
	{
		m->RegistExpand(0, "shadow", "SHADOW_CIRCLE", 0x36f);   // FUN_8001fe58(m, 0, "shadow", SHADOW_CIRCLE, 0x36f)
		if (kingdom::cKingdomModel* sh = m->pExpandMdl(0)) sh->RegistAnm(0u, ShadowClip());   // m +0x60, FUN_8001a4d8(sh, 0, FUN_800c5ce0(), 0, 1)
	}
	return m;
}

void cTaskGimmick::Fx(kingdom::cKingdomPtkHnd& h, uint nSlot, bool bOnetime, bool bDepend, bool bClip, const Vector4& pos, const Vector4* pRot)
{
	// FUN_80021a9c(rate 0, h, slot, onetime, depend, clip, pos, rot, scale 0, light 0, fog -1) -> the app's g02::cTaskEffect slot; natively
	// only while the app has a particle manager (the same convention as cTaskUnit::Fx)
	if (!PtkReady()) return;
	h.Generate(nSlot, bOnetime, bDepend, bClip, 0.0f, pos, pRot != nullptr ? Nullable<Vector4>(*pRot) : Nullable<Vector4>(nullptr), nullptr, 0, -1);
}

void cTaskGimmick::FxOnce(uint nSlot, kingdom::cKingdomModel* m)
{
	if (m == nullptr) return;
	kingdom::cKingdomPtkHnd h;   // the stack handle of the DOL (cJetPtkHnd Init PTR_80393f60 ... dtor DAT_80393f64)
	const Vector4 rot = m->vRotate();
	Fx(h, nSlot, true, false, false, m->vTrans(), &rot);
}

void cTaskGimmick::Se(const char* cue)
{
	WiiSnd::Handle h;
	WiiSnd::Play(h, cue);   // FUN_80129d78(h, cue, 0, 0): no actor
	WiiSnd::Detach(h);
}

// DOL 0x800b1d6c (cTaskConsole::Hooks.GimmickRespawn)
bool GimmickActive(kingdom::cTaskModel* g)
{
	const cTaskGimmick* p = dynamic_cast<const cTaskGimmick*>(g);
	return p != nullptr && p->IsActive();
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the road hazards

// DOL 0x8007352c
cTaskGimmickStone::cTaskGimmickStone()
{
	SetName("g02::cTaskGimmickStone");
	SetupModel(0, "GIMMICK_STONE", nullptr, false);   // 0x52795f9c
}

// DOL 0x80073474
cTaskGimmickOil::cTaskGimmickOil()
{
	SetName("g02::cTaskGimmickOil");
	SetupModel(0, "GIMMICK_OIL", nullptr, false);     // 0xaa1baf48
}

// DOL 0x800733bc
cTaskGimmickSpike::cTaskGimmickSpike()
{
	SetName("g02::cTaskGimmickSpike");
	SetupModel(0, "GIMMICK_SPIKE", nullptr, false);   // 0xa4e1403c
}

namespace
{
const char* const MUD_MODEL[7] = {"GIMMICK_MUD_ONE", "GIMMICK_MUD_RD_0", "GIMMICK_MUD_RD_1", "GIMMICK_MUD_C_0", "GIMMICK_MUD_C_1",
                                  "GIMMICK_MUD_LU_0", "GIMMICK_MUD_LU_1"};
const char* const MUD_NAME[7] = {"Mud ONE", "Mud_0_RIGHT_OR_DOWN", "Mud_1_RIGHT_OR_DOWN", "Mud_0_CENTER", "Mud_1_CENTER", "Mud_0_LEFT_OR_UP",
                                 "Mud_1_LEFT_OR_UP"};
}

// DOL 0x800732d4 / 0x800731ec / 0x80073104 / 0x8007301c / 0x80072f34 / 0x80072e4c / 0x80072d64 (types 4..10)
cTaskGimmickMud::cTaskGimmickMud(int nType)
	: m_nKind(nType >= 4 && nType <= 10 ? nType - 4 : 0)
{
	SetName("g02::cTaskGimmickMud");
	// +0x1f0 cKingdomPtkHnd (Init through the cJetPtkHnd vtable +8): unused, not ported
	SetupModel(0, MUD_MODEL[m_nKind], nullptr, false);
}

// DOL 0x800b22d8 / 0x800b20a4 / 0x800b1e70 / 0x800b2160 / 0x800b1f2c / 0x800b221c / 0x800b1fe8
const char* cTaskGimmickMud::GetName() const { return MUD_NAME[m_nKind]; }

// ---------------------------------------------------------------------------------------------------------------------------------------------
// cTaskGimmickSmoke

// DOL 0x800735e4
cTaskGimmickSmoke::cTaskGimmickSmoke()
{
	SetName("g02::cTaskGimmickSmoke");
	// +0x1f0 cKingdomPtkHnd, +0x244 cKingdomSndActor (FUN_8002654c: m_SndActor)
	InitSmoke();   // vtable +0x3c with the smoke vtable
}

// DOL 0x800b2390
void cTaskGimmickSmoke::InitSmoke()
{
	m_nOwner = 0x20070222;   // no owner
	m_nTime = 0;
	Disable();               // vtable +0x54
}

// DOL 0x800b23f0
bool cTaskGimmickSmoke::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (!IsFrozen() && IsActive() && 0xb4 < m_nTime++) Disable();
	return true;
}

// DOL 0x800b261c
void cTaskGimmickSmoke::Disable()
{
	if (m_Fx.IsValid()) m_Fx.Detach(false);   // FUN_8008a508(+0x1f0, 0)
	cTaskGimmick::Disable();
}

// DOL 0x800b24bc
void cTaskGimmickSmoke::Enable() { Start(0); }

// DOL 0x800b24d0
void cTaskGimmickSmoke::Start(int nIndex)
{
	uint slot = 1;
	const cG02RallyXSys* sys = KingdomGlobal::RallyXSys;   // FUN_800c6428 + FUN_800c5ca0
	if (sys != nullptr && sys->IsMulti())
	{
		if (nIndex == 0) slot = 0xc;
		else if (nIndex == 1) slot = 0xd;
		else if (nIndex == 2) slot = 0xe;
		else if (nIndex == 3) slot = 1;
	}
	Fx(m_Fx, slot, false, false, true, Vector4((float)(m_nX - 4), 0.0f, (float)(m_nZ - 8), 1.0f));
	m_nTime = 0;
	cTaskGimmick::Enable();   // FUN_800b1d60
}

// DOL 0x800b2658
bool cTaskGimmickSmoke::Put(int nOwner, const int pCell[2], int nIndex)
{
	if (IsActive()) return false;
	m_nOwner = nOwner;
	m_nX = pCell[0];
	m_nZ = pCell[1];
	m_SndActor.SetPosition(Vector4((float)(m_nX - 4), 0.0f, (float)(m_nZ - 8), 1.0f));   // 0x800b2738 (velocity 0: FUN_80026660)
	Start(nIndex);            // vtable +0x5c
	m_SndActor.Play("m02se_02Smoke");   // DAT_8047afa8 through FUN_80129d78 on the actor +0x244 (its app-state-6 volume not applied: no app here)
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// cTaskGimmickMan

// the cTaskGimmickMan part of the map Init FUN_80076cb0 (new 0x1c0, cTaskKingdom ctor, +0x1b4..+0x1bc = 0, then the no-op FUN_80079ae8)
cTaskGimmickMan::cTaskGimmickMan()
{
	SetName("g02::cTaskGimmickMan");
}

// DOL 0x800736cc
cTaskGimmick* cTaskGimmickMan::Create(int nType, const int* pCell, int nDir)
{
	cTaskGimmick* g = nullptr;
	switch (nType)   // the factory table 0x80391f48
	{
	case 0: g = new cTaskGimmickSmoke(); break;              // FUN_800735e4
	case 1: g = new cTaskGimmickStone(); break;              // FUN_8007352c
	case 2: g = new cTaskGimmickOil(); break;                // FUN_80073474
	case 3: g = new cTaskGimmickSpike(); break;              // FUN_800733bc
	case 11: g = new cTaskItemFlagNormal(); break;           // FUN_80072ca0
	case 32: g = new cTaskItemFlagLucky(); break;            // FUN_80071414
	case 33: g = new cTaskItemFlagSpecial(); break;          // FUN_80071250
	case 34: g = new cTaskItemFuelBurst(); break;            // FUN_8007113c
	case 35: g = new cTaskItemChameleonPaint(); break;       // FUN_80071028
	case 36: g = new cTaskItemSmokeTank(); break;            // FUN_80070f14
	case 37: g = new cTaskItemSpecialArmor(); break;         // FUN_80070e00
	case 38: g = new cTaskItemMultiFuelBurst(); break;       // FUN_80070c5c
	case 39: g = new cTaskItemMultiChameleonPaint(); break;  // FUN_80070abc
	case 40: g = new cTaskItemMultiSmokeTank(); break;       // FUN_8007091c
	case 41: g = new cTaskItemMultiSpecialArmor(); break;    // FUN_80070708
	default:
		if (nType >= 4 && nType <= 10) g = new cTaskGimmickMud(nType);                 // FUN_800732d4 .. FUN_80072d64
		else if (nType >= 12 && nType <= 31) g = new cTaskItemFlagNumber(nType - 11);   // FUN_80072b20 .. FUN_800714d8
		break;
	}
	if (g == nullptr) return nullptr;
	if (pCell != nullptr)
	{
		g->pCell()[0] = pCell[0];
		g->pCell()[1] = pCell[1];
	}
	g->m_nDir = nDir;     // &+0x1d8 (FUN_800b1cf8)
	g->m_nType = nType;   // &+0x1d4 (FUN_800b1ce4)
	m_vec.push_back(g);
	ConnectChild(g);
	return g;
}

// DOL 0x80073a2c
bool cTaskGimmickMan::Contact(cTaskUnit* pUnit, int x, int z, bool bReact)
{
	bool hit = false;
	// (indexed: a reaction may create gimmicks)
	for (std::size_t i = 0; i < m_vec.size(); i++)
	{
		cTaskGimmick* g = m_vec[i];
		if (g == nullptr || !g->IsActive() || !g->IsAt(x, z)) continue;   // vtable +0x4c
		if (bReact && pUnit != nullptr)
		{
			pUnit->OnGimmick(g, g->nType(), g->nOwner());   // FUN_80085f50(unit, g)
			g->OnHit(pUnit);                                // vtable +0x50
		}
		hit = true;
	}
	return hit;
}

// DOL 0x80073af4
// DOL 0x80073ba0 / 0x80073c24
bool cTaskGimmickMan::IsTypeActive(int nType) const
{
	for (const cTaskGimmick* g : m_vec)
		if (g != nullptr && g->nType() == nType && g->IsActive()) return true;
	return false;
}

bool cTaskGimmickMan::IsCellFree(int x, int z) const
{
	for (const cTaskGimmick* g : m_vec)
		if (g != nullptr && g->IsActive() && g->IsAt(x, z)) return false;
	return true;
}

}
