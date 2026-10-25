#include "kingdom/game/g02/cG02RallyXRecord.hpp"
#include "kingdom/game/g02/cTaskUnit.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "GamePad.hpp"
#include "KingdomGlobal.hpp"
#include "kingdom/cAppKingdom.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cCamera.hpp"
#include "kingdom/game/g02/cG02RallyXSys.hpp"
#include "kingdom/game/g02/cParamCar.hpp"
#include "kingdom/game/g02/cRallyModelShadow.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"
#include "kingdom/game/g02/cTaskMap.hpp"
#include "snd/Mixer.hpp"
#include "wii/Archive.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g02
{

namespace
{
constexpr float DEG = 0.017453292f;          // FLOAT_8047aa24
constexpr float PI = 3.1415927f;             // FLOAT_8047aa50
constexpr float HALF_PI = 1.5707964f;        // FLOAT_8047aa54
constexpr float TWO_PI = 6.2831855f;         // FLOAT_8047aa7c
constexpr float ROLL_MAX = 572.958f;         // FLOAT_8047aa60 (the steering roll +0x254: 10 turns' worth of degrees, read by nothing found)
constexpr float ROLL_STEP = 5.72958f;        // FLOAT_8047aa64

int Rand15() { return std::rand() & 0x7fff; }   // the SDK rand() (0..32767)

// a clip entry's kind (the native IsAttached bookkeeping of 0x8007ca24: CHR0 / VIS0 / CLR0 / PAT0 / SRT0 slots are separate)
int ClipKind(const char* name)
{
	static std::map<std::string, int> cache;
	auto it = cache.find(name);
	if (it != cache.end()) return it->second;
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(name, size);
	int kind = -1;
	static const char* const MAGIC[5] = {"CHR0", "VIS0", "CLR0", "PAT0", "SRT0"};
	if (d != nullptr)
		for (int k = 0; k < 5 && kind < 0; k++)
			if (std::search(d, d + size, MAGIC[k], MAGIC[k] + 4) != d + size) kind = k;
	cache[name] = kind;
	return kind;
}

std::string ClipKey(const kingdom::cKingdomModel* m, uint slot, int kind)
{
	char b[64];
	std::snprintf(b, sizeof b, "%p|%u|%d", (const void*)m, slot, kind);
	return b;
}

bool PtkReady()
{
	kingdom::cKingdomSys* sys = KingdomGlobal::KingSys;
	return sys != nullptr && sys->pAppMain() != nullptr && sys->pAppMain()->pTaskPtkMan() != nullptr;
}

bool IsPlaying(const WiiSnd::Handle& h) { return h.voice >= 0 && snd::IsPlaying(h.voice); }   // FUN_80026424
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// small shared helpers

// DOL 0x800c53c0
bool cTaskUnit::Up(float target, float step, float& v)
{
	v = v + step;
	if (target <= v)
	{
		v = target;
		return true;
	}
	return false;
}

// DOL 0x800c5450
bool cTaskUnit::Down(float target, float step, float& v)
{
	v = v + step;
	if (v <= target)
	{
		v = target;
		return true;
	}
	return false;
}

// DOL 0x800c53ec (FLOAT_8047b2e4 = 1)
bool cTaskUnit::Inc1(float target, float& v) { return Up(target, 1.0f, v); }

// DOL 0x800c547c (FLOAT_8047b2ec = -1)
bool cTaskUnit::Dec1(float target, float& v) { return Down(target, -1.0f, v); }

// DOL 0x800c54ac (FLOAT_8047b2f0 = -99999)
bool cTaskUnit::Dec1Floor(float& v) { return Down(-99999.0f, -1.0f, v); }

int cTaskUnit::DirDeg(int nDir)
{
	switch (nDir)
	{
	case 0: return 0xb4;
	case 1: return 0;
	case 2: return 0x10e;
	case 3: return 0x5a;
	default: return 0;   // (the DOL leaves the register as it was)
	}
}

bool cTaskUnit::IsMultiStage() { return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsMulti(); }
bool cTaskUnit::IsBossStageNow() { return KingdomGlobal::RallyXSys != nullptr && KingdomGlobal::RallyXSys->IsBossStage(); }
int cTaskUnit::SysView(int nIndex) { return (unsigned)nIndex < 4u ? cG02RallyXSysExt::nCtrlType[nIndex] : 2; }   // DOL 0x800c5b88 (sys +0x14 + 0x14 i; ctor default 2)
int cTaskUnit::SysPad(int nIndex) { return (unsigned)nIndex < 4u ? cG02RallyXSysExt::nPadNo[nIndex] : nIndex; }   // DOL 0x800c5b98 (sys +0x1c + 0x14 i; ctor default = the index)
int cTaskUnit::SysPattern() { return 1; }                          // NOT PORTED getter (sys +0x04; ctor default 1)
bool cTaskUnit::SysWinner(int nIndex) { (void)nIndex; return false; }   // NOT PORTED getter (sys +0x10 bit i, set by 0x800c5898)

bool cTaskUnit::IsFrozen() const
{
	const bool home = (GamePad::Get(0).hold & GamePad::PAD_HOME) != 0;
	return home || (m_pHost != nullptr && (m_pHost->IsUnitPause() || m_pHost->IsUnitStageEnd()));
}

int cTaskUnit::CellX() const { return CellOf(m_vPos[0]) + 4; }
int cTaskUnit::CellZ() const { return CellOf(m_vPos[2]) + 8; }

int cTaskUnit::MapCell(int x, int z) const
{
	cTaskMap* map = m_pHost != nullptr ? m_pHost->pUnitMap() : nullptr;
	return map != nullptr ? map->Cell(x, z) : 0;   // the DOL reads MAPOO_MAP +0x20 + (x + z * width) * 2 unchecked
}

// DOL 0x80079aec
bool cTaskUnit::MapRoad(int x, int z) const
{
	cTaskMap* map = m_pHost != nullptr ? m_pHost->pUnitMap() : nullptr;
	return map != nullptr && (map->Attr(x, z) & 1u) != 0;
}

void cTaskUnit::Se(const char* cue)
{
	WiiSnd::Handle h;
	WiiSnd::Play(h, cue);   // FUN_80129d78(h, cue, 0, actor +0x378): the 3D sound actor is NOT PORTED (plain 2D)
	WiiSnd::Detach(h);
}

kingdom::cKingdomModel* cTaskUnit::pBody()
{
	kingdom::cKingdomModel* car = pModel(0);
	return car != nullptr ? car->pExpandMdl(0) : nullptr;
}

kingdom::cKingdomModel* cTaskUnit::pHead()
{
	kingdom::cKingdomModel* body = pBody();
	return body != nullptr ? body->pExpandMdl(0) : nullptr;
}

kingdom::cKingdomModel* cTaskUnit::pHeadSpin()
{
	kingdom::cKingdomModel* body = pBody();
	return body != nullptr ? body->pExpandMdl(1) : nullptr;   // UNVERIFIED: body +0x6c = expand slot 1 (the red heads' spin model)
}

// DOL 0x8007ca24 (natively: the entry last attached to (model, slot, kind))
bool cTaskUnit::IsAttached(kingdom::cKingdomModel* m, const char* name, uint slot) const
{
	if (m == nullptr) return false;
	auto it = m_Clip.find(ClipKey(m, slot, ClipKind(name)));
	return it != m_Clip.end() && it->second == name;
}

void cTaskUnit::Attach(kingdom::cKingdomModel* m, const char* name, uint slot)
{
	if (m == nullptr || IsAttached(m, name, slot)) return;
	m->RegistAnm(slot, name);   // 0x8001a544(m, slot, crc, 0, 1)
	m_Clip[ClipKey(m, slot, ClipKind(name))] = name;
}

void cTaskUnit::Release(kingdom::cKingdomModel* m, uint slot, int nKind)
{
	if (m == nullptr) return;
	switch (nKind)
	{
	case 1: m->ReleaseAnmVis(slot); break;      // 0x8001ad60
	case 2: m->ReleaseAnmClr(slot); break;      // 0x8001af88
	case 3: m->ReleaseAnmTexPat(slot); break;   // 0x8001b1bc
	default: m->ReleaseAnmChr(slot); break;     // 0x8001ab2c
	}
	m_Clip.erase(ClipKey(m, slot, nKind));
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// construction

// the shared part of FUN_80124e54 / FUN_801255a0: cTaskModel ctor, sound handles, 20 particle handles, Init (vtable +0x3c), +0x1cc, models
cTaskUnit::cTaskUnit(cRallyUnitHost* pHost, int nColour, int nIndex) : m_pHost(pHost)
{
	SetName("g02::cTaskUnit");
	InitUnit();
	m_nIndex = nIndex;
	Models(nColour);
}

// (the DOL deletes tasks through the deleting dtor 0x8006fbf8 after Term; natively non-app tasks never run Term, so the dtor frees what
// Term would)
cTaskUnit::~cTaskUnit()
{
	delete m_pParam;
	m_pParam = nullptr;
	delete m_pCamera;
	m_pCamera = nullptr;
}

// DOL 0x8007e358
void cTaskUnit::InitUnit()
{
	nDrawPriority(0x10003u);                    // vtable +0x30
	m_pParam = new cParamCar();                 // 0x100 bytes, CSV_CAR
	m_pParam->Load(IsMultiStage() ? SysPattern() : -1);
	m_nIndex = 0;
	m_nColour = 0;
	m_nFlags = 0;
	m_pShadow = nullptr;                        // +0x2e0
	// +0x2e4 (the Mii) = 0
	Reset();                                    // vtable +0x48 (the cTaskUnit one while constructing, as in the DOL)
}

// DOL 0x8007f638
void cTaskUnit::Term()
{
	if (m_pParam != nullptr)
	{
		delete m_pParam;
		m_pParam = nullptr;
	}
	if (m_pCamera != nullptr)
	{
		delete m_pCamera;
		m_pCamera = nullptr;
	}
	// +0x2e4 (Mii) and +0x378 (sound actor) are NOT PORTED. (No chain to cTaskModel::Term, as in the DOL.)
}

// DOL 0x8008120c
void cTaskUnit::Models(int nColour)
{
	static const char* const CAR[7] = {nullptr, "CAR_BLUE", "CAR_GREEN", "CAR_YELLOW", "CAR_WHITE", "CAR_RED", "CAR_PAC"};        // 0x8036cee8
	static const char* const BODY[7] = {nullptr, "BODY_BLUE", "BODY_GREEN", "BODY_YELLOW", "BODY_WHITE", "BODY_RED", "BODY_PAC"};  // 0x8036cf08
	static const char* const HEAD_RED[3] = {"HEAD_RED1", "HEAD_RED2", "HEAD_RED3"};                                              // 0x8036cf20
	static const char* const HEAD_RED_SPIN[3] = {"HEAD_RED1_SPIN", "HEAD_RED2_SPIN", "HEAD_RED3_SPIN"};                         // 0x8036cf2c
	m_nColour = nColour;
	m_pCamera = nullptr;
	// the sound actor (+0x378, cKingdomSndActor 0x8002654c with 0 / the index in multiplayer): NOT PORTED
	const int c = nColour >= 0 && nColour < 7 ? nColour : 0;
	kingdom::cKingdomModel* car = CreateModel(0);   // 0x8009991c
	if (CAR[c] != nullptr)
	{
		car->RegistModel(CAR[c], 0x36f);                      // 0x8001f72c
		car->RegistExpand(0, "man_root", BODY[c], 0x36f);     // 0x8001fe58
	}
	kingdom::cKingdomModel* body = car->pExpandMdl(0);
	if (body != nullptr)
	{
		if (c == 6) body->RegistExpand(0, "face", "HEAD_PAC", 0x36f);   // 0x197b2485
		else if (c == 5)
		{
			const int i = std::clamp(m_nIndex, 0, 2);
			body->RegistExpand(0, "face", HEAD_RED[i], 0x36f);
			body->RegistExpand(1, "face", HEAD_RED_SPIN[i], 0x36f);
		}
		// colours 1..4: a Mii head (0x8001508c, 0x80015278 with sys 0x800c5c80 / 0x800c5c90, scale cParamCar +0xc4) on "face": NOT PORTED
	}
	m_pShadow = new cRallyModelShadow(this);    // 0x2b0 (see cRallyModelShadow.hpp)
	ConnectChild(m_pShadow);
	m_fFaceTimer = 0.0f;
	const int n = (int)m_pParam->fBlinkChange;
	m_fFaceWait = m_pParam->fBlinkStability + (float)(n > 0 ? Rand15() % n : 0);   // (the DOL divides by it unchecked)
	Face(0);
	ClipsNormal();
	Blend(1.0f);                                 // FLOAT_8047aa2c: the weights snap
	car->SetLightSetIdx(2);                      // 0x8001e1f8 (LIGHT_MODEL)
	car->SetFogIdx(2);                           // 0x8001e400
	if (HasHead())
		if (kingdom::cKingdomModel* head = pHead())
		{
			head->SetLightSetIdx(2);
			head->SetFogIdx(2);
		}
	// UNVERIFIED: the DOL sets no light set on the body (the expanded body keeps its resource light set)
}

// DOL 0x80081cec
void cTaskUnit::CreateCamera(const s32* pScnLayout)
{
	if (m_pCamera != nullptr || m_nIndex >= 4) return;
	m_pCamera = new cCamera(pScnLayout);         // 0xb0 bytes, 0x80127808
	CameraStart(1, 0);
	// the 3D sound listener (+0x2f0) from the camera matrix (GetCameraMtx 0x80129d30, cParamCar +0x80 / +0x84 / +0x88, user data = the index):
	// NOT PORTED
}

// DOL 0x80081dd0
void cTaskUnit::CameraStart(int nWhich, int bFlag)
{
	if (m_pCamera == nullptr || m_nIndex >= 4) return;
	const int i = m_nIndex;
	if (nWhich == 0)
	{
		const int v = nView();
		if ((unsigned)(v - 2) < 2u) m_pCamera->StartTop(i, m_vPos, m_vDir, bFlag);
		else if (v == 1) m_pCamera->StartQuarter(i, m_vPos, m_vDir, bFlag);
	}
	else if (nWhich == 1) m_pCamera->StartFirst(i, m_vPos, m_vDir);
	else if (nWhich == 2)
	{
		const int v = nView();
		if ((unsigned)(v - 2) < 2u) m_pCamera->StartTop(i, m_vPos, m_vDir, 1);
		else if (v == 1) m_pCamera->StartQuarter(i, m_vPos, m_vDir, 1);
		m_pCamera->StartToGame(i, m_vPos, m_vDir);
	}
}

// the unit part of FUN_80078260: +0x200 = (cell x - 4, 0, cell z - 8, 1), the model there, SetDir (the record's direction 0..3)
void cTaskUnit::SetStart(int nCellX, int nCellZ, int nDir)
{
	m_vPos[0] = (float)(nCellX - 4);
	m_vPos[1] = 0.0f;
	m_vPos[2] = (float)(nCellZ - 8);
	m_vPos[3] = 1.0f;
	if (kingdom::cKingdomModel* car = pModel(0)) car->vTrans() = Vector4(m_vPos[0], 0.0f, m_vPos[2], 1.0f);
	if ((unsigned)nDir < 4u) SetDir(nDir);
	// the sound actor position (FUN_80026620 / FUN_80026660): NOT PORTED
}

// DOL 0x80087c0c
void cTaskUnit::SetDir(int nDir)
{
	m_nStartDir = nDir;
	m_nDir = nDir;
	m_nDirOld = nDir;
}

// DOL 0x80087c1c
void cTaskUnit::SetSpeedLevel(int nLevel)
{
	if (m_pParam != nullptr) m_pParam->SetSpeedLevel(nLevel);
}

// DOL 0x80087c9c
float cTaskUnit::FuelCost() const { return m_pParam->fFuelCost; }

// DOL 0x80087998
float cTaskUnit::NitroFuel() const { return m_pParam->fFuelNitroStart; }

float cTaskUnit::fRelocation(int i) const { return (unsigned)i < 4u ? m_pParam->fRelocation[i] : 0.0f; }

// DOL 0x80088a40
int cTaskUnit::nNitroState() const
{
	int n = m_nBrake == 1 ? 2 : 1;
	if (0.0f < m_fNitro) n = 0;
	return n;
}

// DOL 0x80088934: rock-paper-scissors of the nitro state (0 nitro beats 1 normal, 1 beats 2 brake, 2 beats 0)
int cTaskUnit::Rock(const cTaskUnit* a, const cTaskUnit* b)
{
	const int na = a->nNitroState(), nb = b->nNitroState();
	static const int R[3][3] = {{1, 0, 2}, {2, 1, 0}, {0, 2, 1}};   // [a][b]
	return R[na][nb];
}

// DOL 0x8008868c
void cTaskUnit::StartEngine()
{
	if ((unsigned)m_nDir < 2u)
	{
		WiiSnd::Stop(m_SndEngine);   // 0x80026318(+0x360, 0)
		WiiSnd::Play(m_SndEngine, "m02se_01Engine1");
	}
	else if (m_nDir > 1 && m_nDir < 4)
	{
		WiiSnd::Stop(m_SndEngine);
		WiiSnd::Play(m_SndEngine, "m02se_19Engine2");
	}
}

// DOL 0x80088a70
void cTaskUnit::Clear()
{
	m_fNitro = 0.0f;
	if (m_pCamera != nullptr) m_pCamera->StartClear(m_nIndex);
	Face(3);
	if (m_nColour == 6) Attach(pHead(), "HEAD_PAC_ANIM_CLEAR", 0);   // 0x8cfad28c
	else Attach(pBody(), "BODY_ANIM_WIN", 0);                        // 0x2d5cb7ba
	m_nSteer = 1;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// reset

// DOL 0x8007eb80
void cTaskUnit::Reset()
{
	m_bStop = false;
	m_fBlink = 0.0f;
	m_fCellPos = 1.0f;
	const float zero4[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	std::memcpy(m_vPos, zero4, sizeof zero4);
	std::memcpy(m_vOffset, zero4, sizeof zero4);
	std::memcpy(m_vTilt, zero4, sizeof zero4);
	m_nReqTurn = 1;
	m_nSteer = 1;
	m_fTurnRest = 0.0f;
	m_fTurnTotal = 0.0f;
	m_nDirOld = 0;
	m_nDir = 0;
	m_vDir[0] = m_vDir[1] = m_vDir[2] = m_vDir[3] = 0.0f;
	m_fSpinTime = 0.0f;
	m_nSpin = 0;
	m_vDir[1] = DEG * (float)DirDeg(0);         // the direction switch on a constant 0: pi
	std::memset(m_nHit, 0, sizeof m_nHit);
	m_fSpeedGoal = 0.0f;
	m_fSpeed = 0.0f;
	m_nBrake = 0;
	m_fNitro = 0.0f;
	m_fFuel = 100.0f;                           // FLOAT_8047aa44
	m_fFuelUse = 0.0f;
	m_fSmokeTime = 0.0f;
	m_nSmokeLeft = 0;
	m_fFuelBurst = 0.0f;
	m_fChameleon = 0.0f;
	m_nArmor = 0;
	m_nSmokeStock = 5;
	m_fMud = 0.0f;
	m_bSpecial = 0;
	m_nFlagChain = 0;
	Face(2);
	Face(0);
	if (kingdom::cKingdomModel* car = pModel(0))
	{
		ClipsNormal();
		// every CHR slot 0..3 starts at a random frame: rand() / frame count (int), wrapped by the looping clip (UNVERIFIED wrap)
		auto randomFrames = [](kingdom::cKingdomModel* m) {
			if (m == nullptr) return;
			for (uint s = 0; s < 4; s++)
			{
				const float n = m->GetFrameMaxAnmChr(s);
				if (n < 1.0f) continue;
				const int fn = (int)n;
				m->SetFrameAnmChr(s, (float)((Rand15() / fn) % fn));
			}
		};
		randomFrames(car);
		randomFrames(pBody());
		if (HasHead()) randomFrames(pHead());
		Blend(1.0f);                             // FLOAT_8047aa2c
		Release(car, 0, 2);                      // the colour clip of slot 0 if one is attached (0x8001af88)
	}
	m_fStartWait = 0.0f;
	if (!IsBossStageNow() && m_nColour == 5) m_fStartWait = m_pParam->fStartWait;   // the red cars wait at the start
	m_bCrashEnd = false;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// clips

void cTaskUnit::ClipsNormal()
{
	if (m_nColour == 6)
	{
		kingdom::cKingdomModel* car = pModel(0);
		Attach(car, "CAR_PAC_ANIM_NORMAL", 0);   // 0x47a86811
		Attach(car, "CAR_PAC_ANIM_LEFT", 1);     // 0xf5028ff2
		Attach(car, "CAR_PAC_ANIM_RIGHT", 2);    // 0x9ff14aee
		if (HasHead())
		{
			kingdom::cKingdomModel* head = pHead();
			Attach(head, "HEAD_PAC_ANIM_SPEAK", 0);   // 0x92be05e9
			Attach(head, "HEAD_PAC_ANIM_LEFT", 1);    // 0x378693db
			Attach(head, "HEAD_PAC_ANIM_RIGHT", 2);   // 0xdd81569e
		}
	}
	else if (m_nColour > 0 && m_nColour < 6)
	{
		kingdom::cKingdomModel* car = pModel(0);
		Attach(car, "CAR_ANIM_NORMAL", 0);       // 0x15e447e0
		Attach(car, "CAR_ANIM_LEFT", 1);         // 0x8c669a01
		Attach(car, "CAR_ANIM_RIGHT", 2);        // 0xbb3c8d5d
		kingdom::cKingdomModel* body = pBody();
		Attach(body, "BODY_ANIM_NORMAL", 0);     // 0xa1cda4e1
		Attach(body, "BODY_ANIM_LEFT", 1);       // 0xd346dfb7
		Attach(body, "BODY_ANIM_RIGHT", 2);      // 0x9961bba1
	}
}

// DOL 0x8007ded0
void cTaskUnit::BlendNormal(float rate)
{
	kingdom::cKingdomModel* car = pModel(0);
	kingdom::cKingdomModel* body = pBody();
	float w1 = car != nullptr ? car->GetWeightAnmChr(1) : 0.0f;    // 0x8001da38
	float w2 = car != nullptr ? car->GetWeightAnmChr(2) : 0.0f;
	float b1 = body != nullptr ? body->GetWeightAnmChr(1) : 0.0f;
	float b2 = body != nullptr ? body->GetWeightAnmChr(2) : 0.0f;
	Down(0.0f, -rate, w1);
	Down(0.0f, -rate, w2);
	Down(0.0f, -rate, b1);
	Down(0.0f, -rate, b2);
	if (car != nullptr)
	{
		car->SetWeightAnmChr(0, 1.0f);           // 0x8001d994
		car->SetWeightAnmChr(1, w1);
		car->SetWeightAnmChr(2, w2);
		car->SetWeightAnmChr(3, 0.0f);
	}
	if (body != nullptr)
	{
		body->SetWeightAnmChr(0, 1.0f);
		body->SetWeightAnmChr(1, b1);
		body->SetWeightAnmChr(2, b2);
		body->SetWeightAnmChr(3, 0.0f);
	}
}

// DOL 0x8007e0d0
void cTaskUnit::BlendNormalPac(float rate)
{
	kingdom::cKingdomModel* car = pModel(0);
	kingdom::cKingdomModel* head = HasHead() ? pHead() : nullptr;
	float w1 = car != nullptr ? car->GetWeightAnmChr(1) : 0.0f;
	float w2 = car != nullptr ? car->GetWeightAnmChr(2) : 0.0f;
	float h1 = head != nullptr ? head->GetWeightAnmChr(1) : 0.0f;
	float h2 = head != nullptr ? head->GetWeightAnmChr(2) : 0.0f;
	Down(0.0f, -rate, w1);
	Down(0.0f, -rate, w2);
	Down(0.0f, -rate, h1);
	Down(0.0f, -rate, h2);
	if (car != nullptr)
	{
		car->SetWeightAnmChr(0, 1.0f);
		car->SetWeightAnmChr(1, w1);
		car->SetWeightAnmChr(2, w2);
		car->SetWeightAnmChr(3, 0.0f);
	}
	if (head != nullptr)
	{
		head->SetWeightAnmChr(0, 1.0f);
		head->SetWeightAnmChr(1, h1);
		head->SetWeightAnmChr(2, h2);
		head->SetWeightAnmChr(3, 0.0f);
	}
}

// the weight steps of the tilt (0x80082680), TurnLeft (0x800862f0) and TurnRight (0x8008678c): left = slot 1 up to 1 and slot 2 down to 0
// (right the other way) for the car and, by colour, the Pac-Man head (colour 6) or the body (colours 1..5)
void cTaskUnit::BlendTurn(float rate, bool bLeft, bool bHeadByColour)
{
	(void)bHeadByColour;
	kingdom::cKingdomModel* car = pModel(0);
	kingdom::cKingdomModel* sub = m_nColour == 6 ? (HasHead() ? pHead() : nullptr) : pBody();
	float a1 = car != nullptr ? car->GetWeightAnmChr(1) : 0.0f;
	float a2 = car != nullptr ? car->GetWeightAnmChr(2) : 0.0f;
	float s1 = sub != nullptr ? sub->GetWeightAnmChr(1) : 0.0f;
	float s2 = sub != nullptr ? sub->GetWeightAnmChr(2) : 0.0f;
	if (bLeft)
	{
		Up(1.0f, rate, a1);
		Down(0.0f, -rate, a2);
		Up(1.0f, rate, s1);
		Down(0.0f, -rate, s2);
	}
	else
	{
		Down(0.0f, -rate, a1);
		Up(1.0f, rate, a2);
		Down(0.0f, -rate, s1);
		Up(1.0f, rate, s2);
	}
	if (car != nullptr)
	{
		car->SetWeightAnmChr(1, a1);
		car->SetWeightAnmChr(2, a2);
	}
	if (sub != nullptr)
	{
		sub->SetWeightAnmChr(1, s1);
		sub->SetWeightAnmChr(2, s2);
	}
}

// DOL 0x8007d0cc
bool cTaskUnit::IsSpinClip(int nPhase)
{
	static const char* const PAC_L[3] = {"HEAD_PAC_ANIM_LSPIN_S", "HEAD_PAC_ANIM_LSPIN", "HEAD_PAC_ANIM_LSPIN_E"};   // 0x0d3311c1 ..
	static const char* const PAC_R[3] = {"HEAD_PAC_ANIM_RSPIN_S", "HEAD_PAC_ANIM_RSPIN", "HEAD_PAC_ANIM_RSPIN_E"};
	static const char* const BODY_L[3] = {"BODY_ANIM_LSPIN_S", "BODY_ANIM_LSPIN", "BODY_ANIM_LSPIN_E"};
	static const char* const BODY_R[3] = {"BODY_ANIM_RSPIN_S", "BODY_ANIM_RSPIN", "BODY_ANIM_RSPIN_E"};
	if ((unsigned)nPhase > 2u) return false;
	if (m_nColour == 6)
	{
		kingdom::cKingdomModel* head = pHead();
		return IsAttached(head, PAC_L[nPhase], 0) || IsAttached(head, PAC_R[nPhase], 0);
	}
	kingdom::cKingdomModel* body = pBody();
	return IsAttached(body, BODY_L[nPhase], 0) || IsAttached(body, BODY_R[nPhase], 0);
}

// DOL 0x8007d74c (nSide 0 left, 1 right; nPhase 0 start, 1 loop, 2 end)
void cTaskUnit::SpinClip(int nSide, int nPhase)
{
	Face(nPhase == 2 ? 0 : 2);
	static const char* const PAC[2][3] = {{"HEAD_PAC_ANIM_LSPIN_S", "HEAD_PAC_ANIM_LSPIN", "HEAD_PAC_ANIM_LSPIN_E"},
	                                      {"HEAD_PAC_ANIM_RSPIN_S", "HEAD_PAC_ANIM_RSPIN", "HEAD_PAC_ANIM_RSPIN_E"}};
	static const char* const BODY[2][3] = {{"BODY_ANIM_LSPIN_S", "BODY_ANIM_LSPIN", "BODY_ANIM_LSPIN_E"},
	                                       {"BODY_ANIM_RSPIN_S", "BODY_ANIM_RSPIN", "BODY_ANIM_RSPIN_E"}};
	if ((unsigned)nSide > 1u || (unsigned)nPhase > 2u) return;
	if (m_nColour == 6) Attach(pHead(), PAC[nSide][nPhase], 0);
	else Attach(pBody(), BODY[nSide][nPhase], 0);
}

// DOL 0x8007d328
bool cTaskUnit::Face(int nFace)
{
	if (m_nFace == nFace) return false;
	m_nFace = nFace;
	kingdom::cKingdomModel* head = pHead();
	switch (nFace)
	{
	case 0:
		if (m_nColour == 5)
		{
			Release(head, 0, 3);                     // the pattern clip of slot 0 if attached (0x8001b1bc)
			if (head != nullptr) head->Visible();     // 0x8001a1f4
			if (kingdom::cKingdomModel* spin = pHeadSpin()) spin->Invisible();   // 0x8001a330
		}
		else if (m_nColour == 6) Release(head, 0, 3);
		break;
	case 1:
		if (m_nColour == 6) Attach(head, "HEAD_PAC_TEXANI_BLINK", 0);   // 0x691e52d1
		else if (m_nColour == 5) Attach(head, "HEAD_TEXANI_BLINK", 0);  // 0xcb2b3501
		break;
	case 2:
		if (m_nColour == 5)
		{
			if (head != nullptr) head->Invisible();
			if (kingdom::cKingdomModel* spin = pHeadSpin()) spin->Visible();
			Release(head, 0, 3);
		}
		else if (m_nColour == 6) Attach(head, "HEAD_PAC_TEXANI_SPIN", 0);   // 0xea633510
		break;
	case 3:
		if (m_nColour == 6) Attach(head, "HEAD_PAC_TEXANI_SMILE", 0);      // 0x59acc403
		break;
	default: break;
	}
	return true;
}

// DOL 0x8007dd14
bool cTaskUnit::WinClip()
{
	Face(3);
	if (m_nColour == 6) Attach(pHead(), "HEAD_PAC_ANIM_CLEAR", 0);
	else Attach(pBody(), "BODY_ANIM_WIN", 0);
	return true;
}

// DOL 0x8007de2c
bool cTaskUnit::LoseClip()
{
	if (m_nColour == 6) return false;
	Attach(pBody(), "BODY_ANIM_LOSE", 0);   // 0x83d49c87
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the step

// DOL 0x80084d1c
void cTaskUnit::StepMove()
{
	Fuel();
	Speed();
	Move();
	Dec1(0.0f, m_fSmokeTime);
	Collision();
}

// DOL 0x80084d6c
void cTaskUnit::Step()
{
	if (IsFrozen()) return;
	const int st = m_pHost != nullptr ? m_pHost->nUnitGameState() : 0;
	switch (st)
	{
	case 4:
		if (Dec1(0.0f, m_fStartWait))
		{
			Fuel();
			Speed();
			Move();
			Dec1(0.0f, m_fSmokeTime);
			Collision();
		}
		break;
	case 7:
	case 10:
		// FUN_80026394(0, +0x36c, 0): the fuel alarm's volume to 0 (NOT PORTED: no volume control)
		[[fallthrough]];
	case 5:
	case 6:
	case 0xc:
		if (!IsMultiStage())
		{
			m_nSteer = 1;
			Speed();
			Move();
		}
		break;
	default:
		// FUN_80026394(0.1, +0x36c, 500): the fuel alarm's volume (NOT PORTED)
		break;
	}
	SpinStep();
	Heading();
	if (st == 4 && m_fStartWait == 0.0f && m_nSpin == 0) Decide();   // vtable +0x4c
	if (m_nSpin == 0)
	{
		if (m_fTurnRest == 0.0f)
		{
			if (!IsSpinClip(1))
			{
				if (!IsMultiStage())
				{
					if (m_nFace == 1)
					{
						kingdom::cKingdomModel* head = pHead();
						// 0x8001c8f4 / 0x8001c4d8: the blink pattern ended (max <= frame: the port's clips stop at max)
						const bool end = head != nullptr && head->GetFrameMaxAnmTexPat(0) <= head->GetFrameAnmTexPat(0);
						if (end) Face(0);
					}
					if (m_nFace == 0)
					{
						Inc1(m_fFaceWait, m_fFaceTimer);
						if (m_fFaceTimer == m_fFaceWait)
						{
							Face(1);
							m_fFaceTimer = 0.0f;
							const int n = (int)m_pParam->fBlinkChange;
							m_fFaceWait = m_pParam->fBlinkStability + (float)(n > 0 ? Rand15() % n : 0);
						}
					}
				}
				const int st2 = m_pHost != nullptr ? m_pHost->nUnitGameState() : 0;
				bool done = false;
				if (st2 != 0xb)
				{
					if (st2 < 0xb)
					{
						if (st2 > 4)
						{
							Blend(m_pParam->fBlendNormal);   // (colour 6: the float argument is lost in the decomp; UNVERIFIED same +0x94)
							done = true;
						}
					}
					else if (st2 < 0xd) done = true;
				}
				if (!done)
				{
					if (m_fNitro <= 0.0f)
					{
						if (m_fSpeed == 0.0f)
						{
							if (m_nColour == 6) Attach(pModel(0), "CAR_PAC_ANIM_STAY", 0);   // 0xd16ceb06
							else if (m_nColour > 0 && m_nColour < 6) Attach(pModel(0), "CAR_ANIM_STAY", 0);   // 0xa808fef5
						}
						else ClipsNormal();
					}
					else if (m_nColour == 6)
					{
						Attach(pModel(0), "CAR_PAC_ANIM_DASH", 0);   // 0x140bb59a
						if (HasHead()) Attach(pHead(), "HEAD_PAC_ANIM_DASH", 0);   // 0xd68fa9b3
					}
					else if (m_nColour > 0 && m_nColour < 6)
					{
						Attach(pModel(0), "CAR_ANIM_DASH", 0);       // 0x6d6fa069
						Attach(pBody(), "BODY_ANIM_DASH", 0);        // 0x324fe5df
					}
				}
			}
			else ClipsNormal();   // the spin loop is still attached: back to the normal clips
		}
	}
	else
	{
		SpinClip(0, 1);
		Blend(m_pParam->fBlendNormal);
	}
	UpdateFrame();                               // 0x8009a478 cTaskModel::UpdateFrame
	if (m_pCamera != nullptr) m_pCamera->SetSpeedCurve(m_fTurnRest == 0.0f ? 1 : 0);
	// the 3D sound listener (+0x2f0) from the camera eye to the car (cParamCar +0x8c), the actor position: NOT PORTED
}

// DOL 0x800837b4
void cTaskUnit::Fuel()
{
	if (m_fFuelBurst <= 0.0f)
	{
		if (m_fNitro == m_pParam->fNitroTime) m_fFuelUse = m_pParam->fFuelCostNitro;
		else m_fFuelUse = FuelCost();            // vtable +0x54
	}
	else m_fFuelUse = 0.0f;
	Down(0.0f, -FuelUse(), m_fFuel);             // vtable +0x94
	if (!IsMultiStage())
	{
		if (5.0f < m_fFuel || m_fFuel <= 0.0f) WiiSnd::Stop(m_SndFuel);   // FLOAT_8047aa74; 0x80026318(+0x36c, 0)
		else if (!IsPlaying(m_SndFuel)) WiiSnd::Play(m_SndFuel, "m02se_16Fuel");
	}
}

// DOL 0x800838bc
void cTaskUnit::Speed()
{
	Dec1(0.0f, m_fNitro);
	const cParamCar& p = *m_pParam;
	if (m_nSpin == 0)
	{
		if (m_nBrake == 1) m_fSpeedGoal = p.fSpeedBrake;
		else if (m_fNitro <= 0.0f) m_fSpeedGoal = 0.0f < m_fFuel ? p.fSpeedBase : p.fSpeedEmpty;
		else m_fSpeedGoal = p.fSpeedNitro;
	}
	else if ((unsigned)(m_nSpin - 3) < 4u) m_fSpeedGoal = 0.0f;
	else if (m_nSpin == 1) m_fSpeedGoal = p.fSpeedOil;
	else if (m_nSpin == 2) m_fSpeedGoal = p.fSpeedSpike;
	if (m_bCrashEnd) m_fSpeedGoal = 0.0f;
	if (m_bStop) m_fSpeedGoal = 0.0f;
	if (m_fSpeedGoal < m_fSpeed) Down(m_fSpeedGoal, -p.fSpeedSub, m_fSpeed);
	else if (m_fSpeed < m_fSpeedGoal) Up(m_fSpeedGoal, p.fSpeedAdd, m_fSpeed);
}

// DOL 0x80082680
void cTaskUnit::Move()
{
	// M = Trans(pos) * RotY(heading of +0x22c) * Trans(0, 0, speed): the new position is its translation
	const float a = DEG * (float)DirDeg(m_nDir);
	const float sa = std::sin(a), ca = std::cos(a);
	float nx = m_vPos[0] + sa * m_fSpeed;
	const float ny = m_vPos[1];
	float nz = m_vPos[2] + ca * m_fSpeed;
	const float mx = nx, mz = nz;                // the model keeps using M (the unsnapped position)
	const int dirBefore = m_nDir;
	CellCentre();                                // 0x80081f20
	if (dirBefore != m_nDir)
	{
		nx = (float)CellOf(nx);                  // a turn snaps onto the cell centre
		nz = (float)CellOf(nz);
		StartEngine();                           // (inlined: the engine loop by the axis of the new direction)
	}
	const int oldX = CellX(), oldZ = CellZ();
	m_vPos[0] = nx;
	m_vPos[1] = ny;
	m_vPos[2] = nz;
	m_vPos[3] = 1.0f;
	const int newX = CellOf(nx) + 4, newZ = CellOf(nz) + 8;
	const cParamCar& p = *m_pParam;
	switch (m_nSteer)
	{
	case 2:   // left
		Up(p.fDirectionMax, p.fDirectionSpeed, m_vOffset[0]);
		Up(ROLL_MAX, ROLL_STEP, m_vTilt[1]);
		BlendTurn(p.fBlendCtrl, true, true);
		break;
	case 1:   // straight
		if (0.0f <= m_vOffset[0])
		{
			if (0.0f < m_vOffset[0]) Down(-0.0f, -p.fDirectionSpeed, m_vOffset[0]);   // FLOAT_8047aa70
		}
		else Up(0.0f, p.fDirectionSpeed, m_vOffset[0]);
		if (m_fTurnRest == 0.0f) Blend(p.fBlendNormal);
		break;
	case 3:   // right
		Down(-p.fDirectionMax, -p.fDirectionSpeed, m_vOffset[0]);
		Down(-ROLL_MAX, -ROLL_STEP, m_vTilt[1]);
		BlendTurn(p.fBlendCtrl, false, true);
		break;
	default: break;
	}
	m_vOffset[1] = MapCell(newX, newZ) == 8 ? p.fSpikeY : 0.0f;   // the spike bump
	// the model: M * Trans(+0x210) (* the +0x250 rotations, which do not move the translation)
	if (kingdom::cKingdomModel* car = pModel(0))
	{
		const float ox = m_vOffset[0], oy = m_vOffset[1], oz = m_vOffset[2];
		car->vTrans() = Vector4(mx + ca * ox + sa * oz, ny + oy, mz - sa * ox + ca * oz, 1.0f);
	}
	if (oldX != newX || oldZ != newZ)
	{
		m_bSpike = false;
		const int nNew[2] = {newX, newZ}, nOld[2] = {oldX, oldZ};
		OnCellChange(nNew, nOld);                // vtable +0x8c
	}
	if (0.0f < m_fSmokeTime && 0 < m_nSmokeLeft && m_pHost != nullptr && m_pHost->MapPutSmoke((int)nUnitId(), m_vPos, m_nIndex)) m_nSmokeLeft--;
	const int st = m_pHost != nullptr ? m_pHost->nUnitGameState() : 0;
	bool contact = st == 4;
	if (IsBossStageNow() && st == 2) contact = true;
	if (contact && m_pHost != nullptr) m_pHost->MapContact(this, newX, newZ);   // FUN_80079b50 -> the gimmicks under the car
}

// DOL 0x80081f20
void cTaskUnit::CellCentre()
{
	Up(2.0f, m_fSpeed, m_fCellPos);              // FLOAT_8047aa58
	if (Inc1(180.0f, m_fHitClear)) std::memset(m_nHit, 0, sizeof m_nHit);   // FLOAT_8047aa5c
	if (1.0f <= m_fCellPos)
	{
		m_fCellPos -= 1.0f;
		Bump(m_nReqTurn, 0);
		m_nReqTurn = 1;
		int x = CellOf(m_vPos[0]) + 4, z = CellOf(m_vPos[2]) + 8;
		switch (m_nDir)
		{
		case 0: z -= 1; break;
		case 1: z += 1; break;
		case 2: x -= 1; break;
		case 3: x += 1; break;
		default: break;
		}
		if (MapCell(x, z) == 4)
		{
			if (m_pHost != nullptr && m_pHost->nUnitGameState() == 7) m_bCrashEnd = true;
			OnStone();                           // vtable +0x5c
		}
		OnCellCentre();                          // vtable +0x90
	}
}

// DOL 0x80086f58
bool cTaskUnit::CanLeft(int bStone) const
{
	int x = CellX(), z = CellZ();
	switch (m_nDir)
	{
	case 0: x -= 1; break;
	case 1: x += 1; break;
	case 2: z += 1; break;
	case 3: z -= 1; break;
	default: break;
	}
	bool ok = MapRoad(x, z);
	if (bStone != 0 && MapCell(x, z) == 4) ok = false;
	return ok;
}

// DOL 0x800870dc
bool cTaskUnit::CanRight(int bStone) const
{
	int x = CellX(), z = CellZ();
	switch (m_nDir)
	{
	case 0: x += 1; break;
	case 1: x -= 1; break;
	case 2: z -= 1; break;
	case 3: z += 1; break;
	default: break;
	}
	bool ok = MapRoad(x, z);
	if (bStone != 0 && MapCell(x, z) == 4) ok = false;
	return ok;
}

// DOL 0x80087260
bool cTaskUnit::CanBack(int bStone) const
{
	int x = CellX(), z = CellZ();
	switch (m_nDir)
	{
	case 0: z += 1; break;
	case 1: z -= 1; break;
	case 2: x += 1; break;
	case 3: x -= 1; break;
	default: break;
	}
	bool ok = MapRoad(x, z);
	if (bStone != 0 && MapCell(x, z) == 4) ok = false;
	return ok;
}

// DOL 0x80082120
int cTaskUnit::Bump(int nKind, int bStone)
{
	bool ok = false;
	if (nKind == 3) ok = CanRight(bStone);
	else if (nKind == 2) ok = CanLeft(bStone);
	else if (nKind == 4) ok = CanBack(bStone);
	if (!ok)
	{
		int x = CellX(), z = CellZ();
		switch (m_nDir)
		{
		case 0: z -= 1; break;
		case 1: z += 1; break;
		case 2: x -= 1; break;
		case 3: x += 1; break;
		default: break;
		}
		bool road = MapRoad(x, z);
		if (bStone != 0 && MapCell(x, z) == 4) road = false;
		if (road) nKind = 1;
		else if (CanRight(bStone)) nKind = 3;
		else if (CanLeft(bStone)) nKind = 2;
		else if (CanBack(bStone)) nKind = 4;
		// (none possible: the requested kind stays, as in the DOL)
		if (nKind != 1)
		{
			FxOff(m_FxTurn, true);               // FUN_8008a508(+0x4c0, 1)
			Fx(m_FxTurn, 4, true, false, true, Vector4(m_vPos[0], m_vPos[1], m_vPos[2], m_vPos[3]));
		}
	}
	else OnTurn(nKind);                          // vtable +0x50
	if (nKind == 3) TurnRight();
	else if (nKind == 2) TurnLeft();
	else if (nKind == 4) TurnBack();
	return nKind;
}

// DOL 0x800862f0
void cTaskUnit::TurnLeft()
{
	if (m_fTurnRest == 0.0f) m_nDirOld = m_nDir;
	switch (m_nDir)
	{
	case 0: m_nDir = 2; break;
	case 3: m_nDir = 0; break;
	case 1: m_nDir = 3; break;
	case 2: m_nDir = 1; break;
	default: break;
	}
	m_fTurnRest += HALF_PI;
	m_fTurnTotal += HALF_PI;
	if (TWO_PI < m_fTurnRest)
	{
		m_fTurnTotal -= TWO_PI;
		m_fTurnRest -= TWO_PI;
	}
	if (m_nSpin != 0) m_fSpinRot += 0.0f <= m_fSpinRot ? HALF_PI : -HALF_PI;
	BlendTurn(m_pParam->fBlendLeft, true, true);
	Se("m02se_04Stear");
}

// DOL 0x8008678c
void cTaskUnit::TurnRight()
{
	if (m_fTurnRest == 0.0f) m_nDirOld = m_nDir;
	switch (m_nDir)
	{
	case 0: m_nDir = 3; break;
	case 3: m_nDir = 1; break;
	case 1: m_nDir = 2; break;
	case 2: m_nDir = 0; break;
	default: break;
	}
	m_fTurnRest += -HALF_PI;                     // FLOAT_8047aa80
	m_fTurnTotal += -HALF_PI;
	if (m_fTurnRest < -TWO_PI)                   // FLOAT_8047aa84
	{
		m_fTurnTotal -= -TWO_PI;
		m_fTurnRest -= -TWO_PI;
	}
	if (m_nSpin != 0)
	{
		if (0.0f <= m_fSpinRot)
		{
			m_fSpinRot += -HALF_PI;
			if (m_fSpinRot < 0.0f) m_fSpinRot += TWO_PI;
		}
		else
		{
			m_fSpinRot += -HALF_PI;
			if (0.0f < m_fSpinRot) m_fSpinRot -= TWO_PI;
		}
	}
	BlendTurn(m_pParam->fBlendRight, false, true);
	Se("m02se_04Stear");
}

// DOL 0x80086c50
void cTaskUnit::TurnBack()
{
	if (m_fTurnRest == 0.0f) m_nDirOld = m_nDir;
	switch (m_nDir)
	{
	case 0: m_nDir = 1; break;
	case 3: m_nDir = 2; break;
	case 1: m_nDir = 0; break;
	case 2: m_nDir = 3; break;
	default: break;
	}
	m_fTurnRest += PI;
	m_fTurnTotal += PI;
	if (m_fTurnRest < -TWO_PI)
	{
		m_fTurnTotal -= -TWO_PI;
		m_fTurnRest -= -TWO_PI;
	}
	if (TWO_PI < m_fTurnRest)
	{
		m_fTurnTotal -= TWO_PI;
		m_fTurnRest -= TWO_PI;
	}
	if (m_nSpin != 0) m_fSpinRot += 0.0f <= m_fSpinRot ? PI : -PI;
	Se("m02se_05Break");
	// players also play it on the remote speaker (FUN_80025b6c with the pad of sys 0x800c5b98): NOT PORTED
}

// DOL 0x800873e4
bool cTaskUnit::TurnBackHalf()
{
	TurnBack();
	m_fCellPos = 1.0f - m_fCellPos;
	return true;
}

// DOL 0x8008241c
void cTaskUnit::Heading()
{
	float heading = DEG * (float)DirDeg(m_nDir);
	if (m_fTurnRest != 0.0f)
	{
		const bool done = 0.0f <= m_fTurnRest ? Down(0.0f, -m_pParam->fRotAdd, m_fTurnRest) : Up(0.0f, m_pParam->fRotAdd, m_fTurnRest);
		if (!done) heading = DEG * (float)DirDeg(m_nDirOld) + (m_fTurnTotal - m_fTurnRest);
		else m_fTurnTotal = 0.0f;
	}
	m_vDir[1] = heading;
	if (kingdom::cKingdomModel* car = pModel(0))
	{
		if (m_nSpin == 0) car->vRotate().y = m_vDir[1];
		else car->vRotate().y = DEG * (float)DirDeg(m_nDir) - m_fSpinRot;
	}
}

// DOL 0x800835a0
void cTaskUnit::SpinStep()
{
	if (m_nSpin == 0) return;
	if (!m_bStop) Dec1(0.0f, m_fSpinTime);
	const float step = DEG * m_pParam->fSpinRotSpeed;
	if (0.0f <= m_fSpinRot) Down(0.0f, -step, m_fSpinRot);
	else Up(0.0f, step, m_fSpinRot);
	if (m_fSpinRot == 0.0f)
	{
		const int kind = m_nSpin;
		const bool over = m_fSpinTime <= 0.0f;
		m_nSpin = 0;
		if (over) Face(0);
		else if (kind == 5) Spin(5);
		else if (kind < 5)
		{
			if (kind == 3) Spin(3);
			else if (kind > 2) Spin(4);
		}
		else if (kind < 7) Spin(6);
		const int v = MapCell(CellX(), CellZ());
		if (v == 8) Spin(2);
		if (v == 5) Spin(1);
	}
}

// DOL 0x80086e50
bool cTaskUnit::Spin(int nKind)
{
	if ((unsigned)m_nSpin < (unsigned)nKind)
	{
		m_nSpin = nKind;
		if (!IsSpinClip(1))
		{
			SpinClip(0, 0);
			Blend(m_pParam->fBlendNormal);
		}
		m_fSpinRot = 0.0f <= m_fTurnRest ? TWO_PI : -TWO_PI;
		Se("m02se_07Spin");
		return true;
	}
	return false;
}

// DOL 0x80088004
bool cTaskUnit::SpinStart(int nKind)
{
	if (!Spin(nKind)) return false;
	m_nSpinCell[0] = CellX();
	m_nSpinCell[1] = CellZ();
	m_nReqTurn = 1;
	m_fNitro = 0.0f;
	const cParamCar& p = *m_pParam;
	switch (m_nSpin)
	{
	case 3: m_fSpinTime = p.fSpinTimeSmoke; break;
	case 1:
	case 2: m_fSpinTime = 0.0f; break;
	case 4:
	case 5:
	case 6:
		m_fSpinTime = p.fSpinTimeClash;
		m_fSpeed = 0.0f;
		break;
	default: break;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// actions (called by the decisions and the event hooks)

// DOL 0x80087424
bool cTaskUnit::Nitro()
{
	if (!(0.0f < m_fFuel || 0.0f < m_fFuelBurst)) return false;
	m_fNitro = m_pParam->fNitroTime;
	m_fSpeed = m_pParam->fSpeedNitro;
	bool winker = false;
	if (m_fSpeed != 0.0f)
	{
		if (m_nSteer == 2)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_LEFT", 0);    // 0x2f897abb
			winker = true;
		}
		else if (m_nSteer == 3)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_RIGHT", 0);   // 0x902b382f
			winker = true;
		}
	}
	if (!winker) Release(pModel(0), 0, 2);
	if (m_fFuelBurst <= 0.0f)
	{
		m_fFuel -= NitroFuel();                  // vtable +0x98
		if (m_fFuel < 0.0f) m_fFuel = 0.0f;
	}
	Se("m02se_03Nitro");                         // (+ the remote speaker for players: NOT PORTED)
	return true;
}

// DOL 0x80087744
bool cTaskUnit::NitroFree()
{
	if (!(0.0f < m_fFuel || 0.0f < m_fFuelBurst)) return false;
	m_fNitro = m_pParam->fNitroTime;
	m_fSpeed = m_pParam->fSpeedNitro;
	bool winker = false;
	if (m_fSpeed != 0.0f)
	{
		if (m_nSteer == 2)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_LEFT", 0);
			winker = true;
		}
		else if (m_nSteer == 3)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_RIGHT", 0);
			winker = true;
		}
	}
	if (!winker) Release(pModel(0), 0, 2);
	return true;
}

// DOL 0x800879a4
bool cTaskUnit::BrakeOn()
{
	m_nBrake = 1;
	Attach(pModel(0), "CAR_CLRANI_BRAKE", 0);    // 0xa83db9d4
	return true;
}

// DOL 0x80087a3c
bool cTaskUnit::BrakeOff()
{
	m_nBrake = 0;
	bool winker = false;
	if (m_fSpeed != 0.0f)
	{
		if (m_nSteer == 2)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_LEFT", 0);
			winker = true;
		}
		else if (m_nSteer == 3)
		{
			Attach(pModel(0), "CAR_CLRANI_WINKERS_RIGHT", 0);
			winker = true;
		}
	}
	if (!winker) Release(pModel(0), 0, 2);
	return true;
}

// DOL 0x80087bd4
bool cTaskUnit::Smoke()
{
	if (m_nSmokeStock == 0) return false;
	m_nSmokeLeft = m_pParam->nSmoke;
	m_fSmokeTime = m_pParam->fSmokeTime;
	m_nSmokeStock--;
	return true;
}

// DOL 0x80088158
bool cTaskUnit::IsSmokeFree() { return m_pHost == nullptr || m_pHost->MapSmokeFree((int)nUnitId()); }

// DOL 0x800881a0
bool cTaskUnit::DropFlag() { return m_pHost != nullptr && m_pHost->MapDropFlag(m_vPos, m_nDir); }

// DOL 0x80087ca8
void cTaskUnit::ReqLeft()
{
	if (m_fCellPos <= 0.8f) return;              // FLOAT_8047aa88
	m_nReqTurn = 2;
}

// DOL 0x80087cc8
void cTaskUnit::ReqRight()
{
	if (m_fCellPos <= 0.8f) return;
	m_nReqTurn = 3;
}

// DOL 0x80087ce8
void cTaskUnit::ReqBack()
{
	if (m_fTurnRest != 0.0f) return;
	m_nReqTurn = 4;
}

// DOL 0x80087d04 (0) / 0x80087dc4 (1) / 0x80087e84 (2) / 0x80087f44 (3): steer toward an absolute direction
bool cTaskUnit::Want(int nDir)
{
	// [wanted][current] -> 0 straight, 2 left, 3 right, 4 back (from the four DOL functions)
	static const int KIND[4][4] = {
		{1, 4, 3, 2},   // want 0: current 1 -> back, 2 -> right, 3 -> left
		{4, 1, 2, 3},   // want 1
		{2, 3, 1, 4},   // want 2
		{3, 2, 4, 1},   // want 3
	};
	if ((unsigned)nDir > 3u || (unsigned)m_nDir > 3u) return false;   // (the DOL returns another function's value here)
	const int k = KIND[nDir][m_nDir];
	if (k == 4)
	{
		if (m_fTurnRest == 0.0f) m_nReqTurn = 4;
	}
	else if (k != 1)
	{
		if (0.8f < m_fCellPos) m_nReqTurn = k;
	}
	m_nSteer = k;
	return true;
}

// DOL 0x80088264
bool cTaskUnit::Crash()
{
	Se("m02se_06Crash");                         // (+ the remote speaker for players: NOT PORTED)
	Face(2);
	const Vector4 pos(m_vPos[0], m_vPos[1], m_vPos[2], m_vPos[3]);
	FxOff(m_FxCrash, true);
	Fx(m_FxCrash, 3, false, false, true, pos);
	FxOff(m_FxCrash2, true);
	Fx(m_FxCrash2, 0x11, true, false, true, pos);
	m_nSmokeLeft = 0;
	m_fSmokeTime = 0.0f;
	SpinStart(6);
	if (m_pHost != nullptr && m_pHost->nUnitGameState() == 4) m_bStop = true;
	// rumble FUN_80021a14(pad, 500): NOT PORTED
	return true;
}

// DOL 0x80084984
void cTaskUnit::Bounce(cTaskUnit* pOther)
{
	const bool same = m_nDir == pOther->m_nDir;
	if (Bump(4, 1) == 4) m_fCellPos = 1.0f - m_fCellPos;
	if (!same)
		if (pOther->Bump(4, 1) == 4) pOther->m_fCellPos = 1.0f - pOther->m_fCellPos;
	auto remember = [](cTaskUnit* u, int id) {
		u->m_fHitClear = 0.0f;
		for (int i = 0; i < 7; i++)
		{
			if (u->m_nHit[i] == id) break;
			if (u->m_nHit[i] == 0)
			{
				u->m_nHit[i] = id;
				break;
			}
		}
	};
	remember(this, (int)pOther->nUnitId());
	remember(pOther, (int)nUnitId());
	SpinStart(4);
	pOther->SpinStart(4);
	if (IsMultiStage())
	{
		// rumble of both pads: NOT PORTED
		if (0 < m_nFlags && DropFlag()) nFlags(m_nFlags - 1);
		if (0 < pOther->m_nFlags && pOther->DropFlag()) pOther->nFlags(pOther->m_nFlags - 1);
	}
}

// DOL 0x80084c58
void cTaskUnit::SmokeHit()
{
	if (m_nSpin != 0) return;
	SpinStart(3);
	const int n = m_pParam->nSmokeDropFlag;
	if (IsMultiStage())
		for (int i = 0; i < n; i++)
			if (0 < m_nFlags && DropFlag()) nFlags(m_nFlags - 1);
}

// DOL 0x800884dc
bool cTaskUnit::ArmorHit()
{
	if (m_pHost != nullptr && m_pHost->nUnitGameState() == 7)
	{
		if (m_nArmor != 0)
		{
			m_nArmor = 0;
			m_fBlink = m_pParam->fVisibleTime;
			// rumble: NOT PORTED
		}
		return true;
	}
	if (m_nArmor == 0) return !(0.0f < m_fBlink);
	m_nArmor = 0;
	m_fBlink = m_pParam->fVisibleTime;
	// rumble: NOT PORTED
	return false;
}

// DOL 0x800881b8
void cTaskUnit::AddScore(int n)
{
	if (m_pHost != nullptr) m_pHost->UnitAddScore(n);
}

// DOL 0x800881c4
bool cTaskUnit::IsState(int n) const { return m_pHost != nullptr && m_pHost->nUnitGameState() == n; }

// DOL 0x800881dc
void cTaskUnit::FlagFx(int nCount, int nSpecial, const Vector4& pos)
{
	if (!IsMultiStage() && m_pHost != nullptr) m_pHost->FxFlagScore(nCount, nSpecial, pos);
}

// DOL 0x80088440
void cTaskUnit::FuelBurstOn() { m_fFuelBurst = m_pParam->fFuelBurst; }
// DOL 0x80088450
void cTaskUnit::FuelBurstStep() { Dec1Floor(m_fFuelBurst); }
// DOL 0x8008847c
void cTaskUnit::ChameleonOn() { m_fChameleon = m_pParam->fChameleon; }
// DOL 0x8008848c
void cTaskUnit::ChameleonStep() { Dec1(0.0f, m_fChameleon); }

// DOL 0x800885bc
void cTaskUnit::MudOn()
{
	if (m_fMud <= 0.0f)
	{
		m_fMud = m_pParam->fMudTime0 + m_pParam->fMudTime1;
		Se("m02se_08Mad");
	}
}

// DOL 0x80088658
void cTaskUnit::MudStep() { Dec1(0.0f, m_fMud); }

// DOL 0x80085f50 (types: cTaskMap.hpp gimmick table 0x80391f48)
bool cTaskUnit::OnGimmick(cTaskGimmick* g, int nType, int nSmokeOwner)
{
	if (g == nullptr) return false;
	switch (nType)
	{
	case 0:
		if (nSmokeOwner != (int)nUnitId() && ArmorHit()) OnSmoke(g);   // vtable +0x60
		break;
	case 1:
		if (IsState(7)) m_bCrashEnd = true;
		OnStone();
		break;
	case 2: OnOil(g); break;
	case 3:
		if (ArmorHit()) OnSpike(g);
		break;
	case 4: case 5: case 6: case 7: case 8: case 9: case 10: OnMud(g); break;
	case 0x20: OnFlagLucky(g); break;
	case 0x21: OnFlagSpecial(g); break;
	case 0x22: case 0x26: OnItemFuel(g); break;
	case 0x23: case 0x27: OnItemPaint(g); break;
	case 0x24: case 0x28: OnItemSmoke(g); break;
	case 0x25: case 0x29: OnItemArmor(g); break;
	default:
		if ((unsigned)(nType - 0xb) < 0x15u) OnFlag(g);
		break;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// car vs car

// DOL 0x80083a14
void cTaskUnit::Collision()
{
	if (m_fStartWait != 0.0f || m_nSpin != 0 || m_pHost == nullptr) return;
	auto inList = [](const cTaskUnit* u, int id) {
		for (int i = 0; i < 7; i++)
			if (u->m_nHit[i] == id) return true;
		return false;
	};
	auto addList = [](cTaskUnit* u, int id) {
		for (int i = 0; i < 7; i++)
		{
			if (u->m_nHit[i] == id) return;
			if (u->m_nHit[i] == 0)
			{
				u->m_nHit[i] = id;
				return;
			}
		}
	};
	auto dist = [](const cTaskUnit* a, const cTaskUnit* b) {
		const float dx = a->m_vPos[0] - b->m_vPos[0], dy = a->m_vPos[1] - b->m_vPos[1], dz = a->m_vPos[2] - b->m_vPos[2];
		return std::sqrt(dx * dx + dy * dy + dz * dz);   // PSVECDistance
	};
	// `loser` turns back and spins from a clash it lost (the armor / the multiplayer nitro game)
	auto knock = [&](cTaskUnit* loser, cTaskUnit* winner) {
		if (loser->Bump(4, 1) == 4) loser->m_fCellPos = 1.0f - loser->m_fCellPos;
		loser->m_fHitClear = 0.0f;
		addList(loser, (int)winner->nUnitId());
		loser->SpinStart(4);
	};
	auto armorUsed = [](cTaskUnit* u) {
		if (u->m_nArmor != 0)
		{
			u->m_nArmor = 0;
			u->m_fBlink = u->m_pParam->fVisibleTime;
			// rumble: NOT PORTED
		}
	};
	for (int i = 0; i < 7; i++)
	{
		cTaskUnit* o = m_pHost->pUnit(i);
		if (o == nullptr) continue;
		const int myId = (int)nUnitId(), oId = (int)o->nUnitId();
		if (myId == oId) continue;
		if ((myId & 0x10) != 0 && o->m_nSpin != 0) continue;
		const float d = dist(this, o);
		if (!(d < 1.0f && o->m_fStartWait == 0.0f)) continue;   // FLOAT_8047aa2c
		if (inList(this, oId) || inList(o, myId)) continue;
		if (m_nArmor == 1)
		{
			knock(o, this);
			if (IsMultiStage())
				for (int k = 0; k < m_pParam->nArmorDropFlag; k++)
					if (0 < o->m_nFlags && o->DropFlag()) o->nFlags(o->m_nFlags - 1);
			armorUsed(this);
		}
		else if (o->m_nArmor == 1)
		{
			knock(this, o);
			if (IsMultiStage())
				for (int k = 0; k < o->m_pParam->nArmorDropFlag; k++)
					if (0 < m_nFlags && DropFlag()) nFlags(m_nFlags - 1);
			armorUsed(o);
		}
		else if (ArmorHit() && o->ArmorHit())
		{
			const bool playerVsEnemy = ((myId & 0x10) != 0 && (oId & 0x20) != 0) || ((myId & 0x20) != 0 && (oId & 0x10) != 0);
			if (!playerVsEnemy)
			{
				if (!IsMultiStage()) Bounce(o);
				else
				{
					m_pHost->UnitMultiHit(m_nIndex);
					m_pHost->UnitMultiHit(o->m_nIndex);
					const int r = Rock(this, o);
					if (r == 1) Bounce(o);
					else if (r == 0)
					{
						knock(o, this);
						// rumble of o: NOT PORTED
						for (int k = 0; k < 2; k++)
							if (0 < o->m_nFlags && o->DropFlag()) o->nFlags(o->m_nFlags - 1);
					}
					else if (r == 2)
					{
						knock(this, o);
						for (int k = 0; k < 2; k++)
							if (0 < m_nFlags && DropFlag()) nFlags(m_nFlags - 1);
					}
				}
			}
			else OnCrash(o);                     // vtable +0x58 (the player crashes into an enemy)
		}
	}
	int near = 0;
	for (int i = 0; i < 7; i++)
	{
		cTaskUnit* o = m_pHost->pUnit(i);
		if (o == nullptr || o->nUnitId() == nUnitId()) continue;
		if (dist(this, o) < 1.0f) near++;
	}
	if (near == 0 && m_nSpin == 0) std::memset(m_nHit, 0, sizeof m_nHit);
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// Execute (the effects)

void cTaskUnit::Fx(kingdom::cKingdomPtkHnd& h, uint nSlot, bool bOnetime, bool bDepend, bool bClip, const Vector4& pos)
{
	// FUN_80021a9c -> the app's g02::cTaskEffect (+0x1bc) slot; natively only while the app has a particle manager
	if (!PtkReady()) return;
	h.Generate(nSlot, bOnetime, bDepend, bClip, 0.0f, pos, nullptr, nullptr, 0, -1);
}

void cTaskUnit::FxOff(kingdom::cKingdomPtkHnd& h, bool b)
{
	if (h.IsValid()) h.Detach(b);
}

Vector4 cTaskUnit::NodePos(const char* node, const Vector4& add)
{
	kingdom::cKingdomModel* car = pModel(0);
	if (car == nullptr) return Vector4(m_vPos[0], m_vPos[1], m_vPos[2], 1.0f);
	float v[3] = {add.x, add.y, add.z};
	if (const wii::Mdl0* mdl = car->pMdl0())
		for (const wii::Mdl0Node& n : mdl->nodes)
			if (n.name == node)
			{
				v[0] += n.translate[0];          // FUN_8001dd54: the node's resource translation (+0x38)
				v[1] += n.translate[1];
				v[2] += n.translate[2];
				break;
			}
	const Vector4 t = car->vTrans();
	const float a = car->vRotate().y;
	const float s = std::sin(a), c = std::cos(a);
	return Vector4(t.x + c * v[0] + s * v[2], t.y + v[1], t.z - s * v[0] + c * v[2], 1.0f);
}

void cTaskUnit::Effects()
{
	kingdom::cKingdomModel* car = pModel(0);
	if (car == nullptr) return;
	const bool moving = m_fSpeed != 0.0f;
	const Vector4 rot = car->vRotate();
	// the wheels: oil splash (cell 5), mud (6 / 7), sand (always while moving; single player only)
	struct Side
	{
		const char* node;
		kingdom::cKingdomPtkHnd* oil;
		kingdom::cKingdomPtkHnd* mud;
		kingdom::cKingdomPtkHnd* sand;
	};
	const Side sides[2] = {{"b_l_spark", &m_FxOilL, &m_FxMudL, &m_FxSandL}, {"b_r_spark", &m_FxOilR, &m_FxMudR, &m_FxSandR}};
	for (const Side& s : sides)
	{
		const Vector4 p = NodePos(s.node, Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		const int v = MapCell(CellOf(p.x) + 4, CellOf(p.z) + 8);
		if (!moving || v != 5) FxOff(*s.oil);
		else
		{
			if (!s.oil->IsValid()) Fx(*s.oil, 5, false, false, true, p);
			s.oil->vSetTrans(p);
		}
		if (!moving || (v != 6 && v != 7)) FxOff(*s.mud);
		else
		{
			if (!s.mud->IsValid()) Fx(*s.mud, 6, false, false, true, p);
			s.mud->vSetTrans(p);
			s.mud->vSetRotate(rot);
		}
		if (!moving) FxOff(*s.sand);
		else
		{
			const Vector4 q = NodePos(s.node, Vector4(0.0f, 0.0f, m_pParam->fSandNodePlus, 0.0f));   // (0, 0, +0xc0)
			if (!s.sand->IsValid() && !IsMultiStage()) Fx(*s.sand, 0x10, false, false, true, q);
			s.sand->vSetTrans(q);
		}
	}
	// the turn sparks
	if (m_fTurnRest == 0.0f)
	{
		FxOff(m_FxSparkL);
		FxOff(m_FxSparkR);
	}
	else if (m_fTurnTotal <= 0.0f)
	{
		if (m_fTurnTotal < 0.0f)
		{
			const Vector4 p = NodePos("b_l_spark", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
			if (!m_FxSparkL.IsValid()) Fx(m_FxSparkL, 7, false, false, true, p);
			m_FxSparkL.vSetTrans(p);
		}
	}
	else
	{
		const Vector4 p = NodePos("b_r_spark", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxSparkR.IsValid()) Fx(m_FxSparkR, 7, false, false, true, p);
		m_FxSparkR.vSetTrans(p);
	}
	// armor
	if (m_nArmor == 1)
	{
		const Vector4 p = NodePos("cart", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxArmor.IsValid()) Fx(m_FxArmor, 8, false, true, true, p);
		m_FxArmor.vSetTrans(p);
	}
	else FxOff(m_FxArmor);
	// chameleon paint
	if (0.0f < m_fChameleon)
	{
		const Vector4 p = NodePos("cart", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxPaint.IsValid()) Fx(m_FxPaint, 0xf, false, true, true, p);
		m_FxPaint.vSetTrans(p);
		const int colour = (nUnitId() & 0x20) != 0 ? 1 : m_nIndex;
		// the car's PB model +0x290 = colour (cTaskModel::pPBModel 0x8009a454): NOT PORTED (read by the draw)
		if (m_pShadow != nullptr) m_pShadow->nColour = colour;
	}
	else
	{
		FxOff(m_FxPaint);
		if (m_pShadow != nullptr) m_pShadow->nColour = -1;
	}
	// nitro
	if (m_fNitro <= 0.0f)
	{
		FxOff(m_FxNitroR);
		FxOff(m_FxNitroL);
		FxOff(m_FxNitro);
	}
	else
	{
		const Vector4 r1(rot.x + HALF_PI, rot.y + PI, rot.z, rot.w);
		Vector4 p = NodePos("r_nitro", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxNitroR.IsValid()) Fx(m_FxNitroR, 2, false, true, true, p);
		m_FxNitroR.vSetTrans(p);
		m_FxNitroR.vSetRotate(r1);
		p = NodePos("l_nitro", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxNitroL.IsValid()) Fx(m_FxNitroL, 2, false, true, true, p);
		m_FxNitroL.vSetTrans(p);
		m_FxNitroL.vSetRotate(r1);
		const Vector4 r2(rot.x, rot.y + PI, rot.z, rot.w);
		p = NodePos("eff", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxNitro.IsValid() && !IsMultiStage())
		{
			Fx(m_FxNitro, 0xb, false, false, true, p);
			m_FxNitro.vSetRotate(rot);           // (generated with the model's rotation)
		}
		m_FxNitro.vSetTrans(p);
		m_FxNitro.vSetRotate(r2);
	}
	// exhaust: smoke while there is fuel, the empty-tank smoke otherwise
	if (0.0f < m_fFuel)
	{
		Vector4 p = NodePos("l_smoke", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxSmokeL.IsValid()) Fx(m_FxSmokeL, 10, false, false, true, p);
		m_FxSmokeL.vSetTrans(p);
		p = NodePos("r_smoke", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxSmokeR.IsValid()) Fx(m_FxSmokeR, 10, false, false, true, p);
		m_FxSmokeR.vSetTrans(p);
		FxOff(m_FxEmptyL);
		FxOff(m_FxEmptyR);
	}
	else
	{
		Vector4 p = NodePos("l_smoke", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxEmptyL.IsValid()) Fx(m_FxEmptyL, 0x2b, false, false, true, p);
		m_FxEmptyL.vSetTrans(p);
		p = NodePos("r_smoke", Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		if (!m_FxEmptyR.IsValid()) Fx(m_FxEmptyR, 0x2b, false, false, true, p);
		m_FxEmptyR.vSetTrans(p);
		FxOff(m_FxSmokeL);
		FxOff(m_FxSmokeR);
	}
}

// DOL 0x8007f714
bool cTaskUnit::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (!Dec1(0.0f, m_fBlink))
	{
		// blinking after the armor took a hit: the visibility clips (natively the VIS clip hides the nodes; the DOL also turns the whole car
		// Invisible / Visible from the clip's node 1 at the end of this function, UNVERIFIED equivalent)
		Attach(pModel(0), "CAR_VIS_VISIBLE_CAR", 0);    // 0x3933843b
		Attach(pBody(), "CAR_VIS_VISIBLE_BODY", 0);     // 0xdf3d23c5
		if (HasHead()) Attach(pHead(), "HEAD_VIS_VISIBLE_HEAD", 0);   // 0x9858b922
	}
	else
	{
		Release(pModel(0), 0, 1);                // 0x8001ad60 when a VIS clip is attached
		Release(pBody(), 0, 1);
		Release(pHead(), 0, 1);
	}
	Effects();
	// the Mii head's visibility (+0x2e4): NOT PORTED
	return true;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------
// the console's view of a car (cTaskConsole::Hooks.CarState; the DOL parts read these unit offsets directly)

bool FillConsCar(kingdom::cTaskModel* pCar, sConsCar& out)
{
	const cTaskUnit* u = dynamic_cast<const cTaskUnit*>(pCar);
	if (u == nullptr) return false;
	out.nKind = u->nUnitId();         // vtable +0x44
	out.fX = u->m_vPos[0];            // +0x200
	out.fZ = u->m_vPos[2];            // +0x208
	out.fWait = u->m_fStartWait;      // +0x1d4
	out.nDir = u->m_nDir;             // +0x22c
	out.nCrash = u->m_nSpin;          // +0x260
	out.fNitro = u->m_fNitro;         // +0x2a4
	out.fFuel = u->m_fFuel;           // +0x2ac
	out.fTimer2bc = u->m_fFuelBurst;  // +0x2bc
	out.fTimer2c0 = u->m_fChameleon;  // +0x2c0
	out.fMud = u->m_fMud;             // +0x2c4
	out.n2c8 = u->m_nArmor;           // +0x2c8
	out.nSmoke = u->m_nSmokeStock;    // +0x2cc
	out.nFlags = u->m_nFlags;         // +0x2d0
	return true;
}

}
