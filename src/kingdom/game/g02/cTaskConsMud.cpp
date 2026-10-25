#include "kingdom/game/g02/cTaskConsMud.hpp"

#include <algorithm>

#include "GamePad.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g02/cTaskConsole.hpp"

// Rally-X Remix (g02) the mud splats on the screen (kingdom::game::g02::cTaskConsMud). Ported from main.dol (no C# exists for Rally-X).
namespace kingdom::game::g02
{

namespace
{
const char* const MUD = "ANITA_MUD";   // 0x6db9f578
constexpr int CLIP_IN[4] = {0, 3, 6, 9};       // 0x8036bd40 [kind]
constexpr int CLIP_STAY[4] = {1, 4, 7, 10};    // 0x8036bd50
constexpr int CLIP_OUT[4] = {2, 5, 8, 11};     // 0x8036bd60
constexpr uint BIG = 0x18;                     // anitas 0 .. 0x17 big, 0x18 .. 0x1b small
constexpr uint SMALL = 4;

// insertion sort of the splat table by the random key, ascending (layer and key move, the kind stays) - the DOL's inlined loop
template <std::size_t N>
void Shuffle(cTaskConsMud::sSplat (&s)[N], cTaskConsole* con)
{
	for (std::size_t i = 0; i < N; i++)
	{
		s[i].nKey = con->Rand();   // 0x8014146c(console +0x1b8)
		for (std::size_t j = i; j > 0; j--)
		{
			const u32 key = s[j].nKey;
			if (s[j - 1].nKey <= key) break;
			std::swap(s[j].nLayer, s[j - 1].nLayer);
			s[j].nKey = s[j - 1].nKey;
			s[j - 1].nKey = key;
		}
	}
}
}

// DOL 0x800632ac (cTaskPBAnita priority 0x8000000, then 0x8000001; see the header for the draw order)
cTaskConsMud::cTaskConsMud(cTaskConsole* pConsole, int nPlayer) : m_pConsole(pConsole)
{
	SetName("g02::cTaskConsMud");
	nDrawPriority(0x80000000u);   // UNVERIFIED stand-in for the push-back priority 0x8000001 (no cTaskPBAnita in the port)
	m_b1b4 = false;
	m_bDraw = true;
	m_nPlayer = nPlayer;
	m_pCar = nullptr;
	m_nMode = 0;
	m_nBig = m_nSmall = 0;
	m_fMaxFrame = 0.0f;
	// the own cParamCar (ctor inlined: every CSV_CAR row; only these four are read by the mud)
	kingdom::cKingdomCSV csv;
	if (csv.RegistData("CSV_CAR"))
	{
		m_fMudTime = 60.0f * csv.F32(0x18, 1, 0.0f);    // +0x70 (FLOAT_8047a8fc = 60)
		m_fFadeTime = 60.0f * csv.F32(0x19, 1, 0.0f);   // +0x74
		m_nCarBig = (uint)csv.S32(0x1a, 1, 0);          // +0x78
		m_nCarSmall = (uint)csv.S32(0x1b, 1, 0);        // +0x7c
	}
	if (m_fMudTime < 13.0f) m_fMudTime = 13.0f;         // FLOAT_8047a908
	for (int i = 0; i < 25; i++) m_Big[i] = sSplat{i, 0u, 0};
	for (int i = 0; i < 13; i++) m_Small[i] = sSplat{0x19 + i, 0u, 0};
	for (uint i = 0; i < BIG; i++)
	{
		Regist(i, MUD, 3);
		pAnita(i)->SetState(1u, false);
	}
	for (uint i = 0; i < SMALL; i++)
	{
		Regist(i + BIG, MUD, 3);
		pAnita(i + BIG)->SetState(1u, false);
	}
	for (uint k = 0; k < 4; k++)   // the longest "out" clip (on anita 0, which keeps clip 11 afterwards)
	{
		pAnita(0)->SetAnimNo(CLIP_OUT[k], true);
		const float n = pAnita(0)->GetFrameNum(-1);   // 0x80141bd4
		if (m_fMaxFrame < n) m_fMaxFrame = n;
	}
}

// DOL 0x80123418
kingdom::cKingdomAnita* cTaskConsMud::Regist(uint idx, const char* name, int anim)
{
	kingdom::cKingdomAnita* a = pAnita(idx);
	a->RegistData(name);
	a->SetAnimNo(anim, true, 0.0f);
	a->vTrans()[0] = 0.0f;
	a->vTrans()[1] = 0.0f;
	a->vTrans()[2] = 0.0f;
	return a;
}

// DOL 0x801233d8
void cTaskConsMud::SetAnim(uint idx, int anim)
{
	pAnita(idx)->SetAnimNo(anim, false, 0.0f);
}

// DOL 0x8012351c
bool cTaskConsMud::SetPosLayer(uint idx, int pat, uint lay)
{
	Vector4 pos{};
	kingdom::cKingdomAnita* a = pAnita(idx);
	if (!a->GetLayerPos(pos, lay, pat)) return false;
	a->vTrans()[0] = pos[0];
	a->vTrans()[1] = pos[1];
	return true;
}

void cTaskConsMud::HideAll()
{
	for (uint i = 0; i < BIG; i++) pAnita(i)->SetState(1u, false);
	for (uint i = 0; i < SMALL; i++) pAnita(i + BIG)->SetState(1u, false);
}

// the mud half of DOL 0x80065db0: the splat counts come from the car's cParamCar (+0x2d4 -> +0x78 / +0x7c), capped at 24 / 4
void cTaskConsMud::SetCar(kingdom::cTaskModel* pCar)
{
	if (pCar == nullptr) return;
	m_pCar = pCar;
	m_nBig = std::min(m_nCarBig, 0x18u);
	m_nSmall = std::min(m_nCarSmall, 4u);
}

// DOL 0x80068bac
bool cTaskConsMud::Execute(cJetTask* pParam)
{
	(void)pParam;
	if (m_pConsole == nullptr || m_pConsole->IsFrozen()) return true;
	if (m_pCar != nullptr)
	{
		sConsCar car;
		cTaskConsole::CarState(m_pCar, car);
		if (m_nMode == 0)
		{
			if (car.fMud > 0.0f)   // 0x80088664
			{
				// big splats: shuffle the 25 layers, a random kind per entry; the first m_nBig entries are placed, kinds 2 / 3 first. The
				// kind is written to the ANITA's entry while the layer comes from the source entry (the DOL mixes the two indexes).
				Shuffle(m_Big, m_pConsole);
				u32 kind[25];
				for (int i = 0; i < 25; i++) kind[i] = m_pConsole->Rand() & 3;
				uint a = 0;
				for (uint pass = 0; pass < 2; pass++)
				{
					for (uint i = 0; i < m_nBig; i++)
					{
						const u32 k = kind[i];
						if ((pass == 0) != (1 < k)) continue;
						pAnita(a)->SetAnimNo(CLIP_IN[k], true);
						SetPosLayer(a, 0, (uint)m_Big[i].nLayer);
						pAnita(a)->SetState(1u, true);
						m_Big[a].nKind = (int)k;
						a++;
					}
				}
				// small splats: shuffle the 13 layers, kind 0 / 1 per entry, the first m_nSmall placed in order
				Shuffle(m_Small, m_pConsole);
				for (int i = 0; i < 13; i++) m_Small[i].nKind = (int)(m_pConsole->Rand() & 1);
				for (uint i = 0; i < m_nSmall; i++)
				{
					pAnita(i + BIG)->SetAnimNo(CLIP_IN[m_Small[i].nKind], true);
					SetPosLayer(i + BIG, 0, (uint)m_Small[i].nLayer);
					pAnita(i + BIG)->SetState(1u, true);
				}
				m_nMode++;
			}
		}
		else if (m_nMode == 1)
		{
			// the car's remaining mud time (+0x2c4, 0x8008867c) at or below the fade time: every splat plays its "out" clip
			if (m_fMudTime <= (m_fMudTime + m_fFadeTime) - car.fMud)
			{
				for (uint i = 0; i < m_nBig; i++) SetAnim(i, CLIP_OUT[m_Big[i].nKind]);
				for (uint i = 0; i < m_nSmall; i++) SetAnim(i + BIG, CLIP_OUT[m_Small[i].nKind]);
				m_nMode++;
			}
			else
			{
				for (uint i = 0; i < m_nBig; i++)
				{
					if (pAnita(i)->IsState(0x10000u)) SetAnim(i, CLIP_STAY[m_Big[i].nKind]);
				}
				for (uint i = 0; i < m_nSmall; i++)
				{
					if (pAnita(i + BIG)->IsState(0x10000u)) SetAnim(i + BIG, CLIP_STAY[m_Small[i].nKind]);
				}
			}
		}
		else if (m_nMode == 2 && pAnita(BIG)->IsState(0x10000u))   // the first small splat's clip ended
		{
			HideAll();
			m_nMode = 0;
		}
		if (m_pConsole->nState() == 3 && m_nMode != 0)   // the retry after a miss
		{
			m_nMode = 0;
			HideAll();
		}
	}
	// every anita steps 1 frame; while fading out (mode 2) by its clip length / the fade time so the "out" clips last CSV_CAR row 0x19 s
	float step = 1.0f;   // FLOAT_8047a90c
	for (uint i = 0; i < BIG + SMALL; i++)
	{
		kingdom::cKingdomAnita* a = pAnita(i);
		if (m_nMode == 2)
		{
			if (m_fFadeTime == 0.0f) step = 1.0f;
			else step = a->GetFrameNum(a->GetAnimNo()) / m_fFadeTime;
		}
		if ((GamePad::Get(0).hold & GamePad::PAD_HOME) == 0 || a->IsHBMUpdate()) a->UpdateFrame(step);   // 0x80141c44
	}
	return true;
}

// DOL 0x801235cc (the push-back draw: an orthographic projection of the render mode around the anita vector draw, only while +0x1d4).
// Equivalent of the cTaskPBAnita path the port does not have: cTaskPushBack::Draw 0x800362b0 -> vtable +0x54 0x8009abe0 (pushes the task
// into the ScnRoot, FUN_8009c3f4) -> the scene calls vtable +0x40 = this; +0x48 0x8003629c returns the push-back depth (FLOAT_8047a014).
void cTaskConsMud::Draw(int nStatus, cJetTask* pParam)
{
	if (!m_bDraw) return;
	cTaskAnita::Draw(nStatus, pParam);
}

}
