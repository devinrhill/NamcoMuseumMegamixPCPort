// kingdom::cTaskFontWriter (natively gfx::DrawText in Draw) (DOL vtable 0x804381c0, 0x26c bytes; ctor inlined in 0x800c0044) as a child task: Init(0x400, 0x200) 0x80016d2c,
//        SetFont(KingSys +0xa68) 0x80016d98, colour 0x80016dd0(-1), 0x80016dd8(18.0f) (UNVERIFIED: font size), 0x80016de0(0), visible byte +0x268,
//        printf-style queue 0x80016e88(x, y, fmt, ...) used by Execute. Until it exists the name is not drawn (state kept in m_bFontVisible / m_vFontPos).
// NEEDS: kingdom::cTaskFaceIcon (vtable 0x80438140, 0x1c8 bytes) + cKingdomFaceIconVector (+0x1b4): icon(0) = 0x80015d58, Mii face texture
//        0x8001572c(icon, player +0x44, player +0x48, 0x80, 0, 0, -1, 0), icon +0x40 position, +0x60 scale, +0x7c visible. Not drawn (state kept).
#include "kingdom/game/g01/cTaskConsMultiMenuPlayer.hpp"

#include <cmath>

#include "gfx/FontWriter.hpp"
#include "gfx/Gfx.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/game/g01/cG01GalagaSys.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom::game::g01
{

namespace
{
const char* const ANT_MENU = "ANT_MENU";   // 0xd3a909fe, g01menu.lzs

const uint WND_LAY[4] = {0u, 1u, 2u, 3u};                 // DOL 0x80370650: anita 0 layer per player (pattern 3) = the panel origin
const uint FACE_LAY_A = 10u;                              // DOL 0x80370660
const uint FACE_LAY_POS = 9u;                             // DOL 0x80370664
const uint FACE_LAY_B = 11u;                              // DOL 0x80370668
const uint SCORE_LAY[6] = {2u, 3u, 4u, 5u, 6u, 7u};       // DOL 0x80370670: anitas 4..9 (pattern 2)
// [player][appear, wait, delete]
const int FRAME1_ANM[4][3] = {{0x4d, 0x4c, 0x4e}, {0xc1, 0xc2, 0xc3}, {0xa3, 0xa4, 0xa5}, {0xa6, 0xa7, 0xa8}};   // DOL 0x80370688 (anita 1)
const int FRAME2_ANM[4][3] = {{0x76, 0x77, 0x78}, {0x79, 0x7a, 0x7b}, {0x7c, 0x7d, 0x7e}, {0x7f, 0x80, 0x81}};   // DOL 0x803706b8 (anita 2)
const int MARK_ANM[3] = {0xc4, 0xc5, 0xc6};               // DOL 0x803706e8 (anita 3)
// DOL 0x803706f8: [digit][appear, wait, delete] = digit * 3 + kind (0..29)
int ScoreAnm(int nDigit, int nKind) { return nDigit * 3 + nKind; }

constexpr uint STATE_VISIBLE = 1u;          // anita +4 bit 0
constexpr uint STATE_ANM_END = 0x10000u;    // anita +4 bit 16

void AddXYZ(Vector4& dst, const Vector4& a, const Vector4& b)   // PSVECAdd (w of dst untouched)
{
	dst.x = a.x + b.x;
	dst.y = a.y + b.y;
	dst.z = a.z + b.z;
}
}

// DOL 0x800c0044
cTaskConsMultiMenuPlayer::cTaskConsMultiMenuPlayer(cTaskConsMenuManBase* const& pConsMan, int nPlayer, char* pName)
	: cTaskConsMenuBase(pConsMan)   // DOL 0x80145474
{
	m_pName = pName;
	m_nPlayer = nPlayer;
	m_nScore = -1;
	m_bMark = false;
	m_nWorld = -1;
	m_nStage = -1;

	// anita 0: the panel; only gives the origin of everything else (layer WND_LAY[player], pattern 3), stays hidden
	// UNVERIFIED: RegistData's third argument (0x8000d53c(KingSys)) is left at the port's default draw layer.
	Vector4 vBase(0.0f, 0.0f, 0.0f);
	pAnita(0u)->RegistData(ANT_MENU);
	pAnita(0u)->SetAnimNo(0, true);
	pAnita(0u)->SetState(STATE_VISIBLE, false);
	pAnita(0u)->GetLayerPos(vBase, WND_LAY[m_nPlayer], 3);

	pAnita(1u)->RegistData(ANT_MENU);
	pAnita(1u)->SetAnimNo(FRAME1_ANM[m_nPlayer][1], true);
	pAnita(1u)->SetState(STATE_VISIBLE, false);

	Vector4 v(0.0f, 0.0f, 0.0f);
	pAnita(2u)->RegistData(ANT_MENU);
	pAnita(2u)->SetAnimNo(FRAME2_ANM[m_nPlayer][1], true);
	pAnita(2u)->SetState(STATE_VISIBLE, false);
	pAnita(2u)->GetLayerPos(v, 0u, 2);
	AddXYZ(pAnita(2u)->vTrans(), v, vBase);

	pAnita(3u)->RegistData(ANT_MENU);
	pAnita(3u)->SetAnimNo(MARK_ANM[1], true);
	pAnita(3u)->SetState(STATE_VISIBLE, false);
	pAnita(3u)->GetLayerPos(v, 1u, 2);
	AddXYZ(pAnita(3u)->vTrans(), v, vBase);

	for (uint i = 4u; i <= 9u; i++)
	{
		pAnita(i)->RegistData(ANT_MENU);
		pAnita(i)->SetAnimNo(ScoreAnm(0, 1), true);
		pAnita(i)->SetState(STATE_VISIBLE, false);
		pAnita(i)->GetLayerPos(v, SCORE_LAY[i - 4u], 2);
		AddXYZ(pAnita(i)->vTrans(), v, vBase);
	}

	// anita 10: anchor of the name (hidden)
	Vector4 vName(0.0f, 0.0f, 0.0f);
	pAnita(10u)->RegistData(ANT_MENU);
	pAnita(10u)->SetAnimNo(0, true);
	pAnita(10u)->SetState(STATE_VISIBLE, false);
	pAnita(10u)->GetLayerPos(vName, 8u, 2);

	// cTaskFontWriter child (NEEDS): priority 0x80000002, ConnectChild, Init(0x400, 0x200), font KingSys +0xa68, colour -1, 18.0f, hidden
	m_bFontVisible = false;
	AddXYZ(m_vFontPos, vName, vBase);
	// 0x80138e58 = the render mode: +4 fbWidth, +6 efbHeight (UNVERIFIED: taken as the 640 x 480 screen)
	m_vFontPos.x = 0.5f * (float)gfx::SCREEN_W + m_vFontPos.x;
	m_vFontPos.y = m_vFontPos.y - 0.5f * (float)gfx::SCREEN_H;

	// anita 11: anchors of the face icon (hidden, no animation set)
	Vector4 vFace(0.0f, 0.0f, 0.0f);
	Vector4 vA(0.0f, 0.0f, 0.0f);
	Vector4 vB(0.0f, 0.0f, 0.0f);
	pAnita(11u)->RegistData(ANT_MENU);
	pAnita(11u)->SetState(STATE_VISIBLE, false);
	pAnita(11u)->GetLayerPos(vFace, FACE_LAY_POS, 2);
	pAnita(11u)->GetLayerPos(vA, FACE_LAY_A, 2);
	pAnita(11u)->GetLayerPos(vB, FACE_LAY_B, 2);
	const float fScale = (vB.x - vA.x) * 0.0078125f;   // / 128 (face texture size)

	// cTaskFaceIcon child (NEEDS): icon 0 hidden, ConnectChild, Mii face (player +0x44, +0x48, 128), priority 0x80000003
	m_bFaceVisible = false;
	AddXYZ(m_vFacePos, vFace, vBase);
	m_vFaceScale.x = std::fabs(fScale);
	m_vFaceScale.y = std::fabs(fScale);
	m_vFaceScale.z = std::fabs(0.0f);
	m_vFaceScale.w = 1.0f;
}

// DOL 0x800c13dc
cTaskConsMultiMenuPlayer::~cTaskConsMultiMenuPlayer()
{
}

// DOL 0x800c0b88
void cTaskConsMultiMenuPlayer::SetRecord(int nWorld, int nStage)
{
	if (nWorld != m_nWorld || nStage != m_nStage)
	{
		m_nWorld = nWorld;
		m_nStage = nStage;
		sMultiRecord* pRec = KingdomGlobal::GalagaSys->pMultiRecord(m_nWorld, m_nStage);   // DOL 0x800afeb4
		m_nScore = pRec->n[10 + m_nPlayer];
		m_bMark = ((u32)pRec->n[0] & (1u << m_nPlayer)) != 0;
	}
}

// DOL 0x800c0c14
void cTaskConsMultiMenuPlayer::SetScoreAnm(int nKind)
{
	const int nScore = m_nScore;
	for (uint i = 4u, n = 0u; i <= 9u; i++, n++)
	{
		int nDiv = 10;
		int nUnit = 1;
		for (uint k = 0u; k < n; k++)
		{
			nDiv *= 10;
			nUnit *= 10;
		}
		const int nDigit = (nScore % nDiv) / nUnit;
		if (nScore < nUnit && n != 0u && nDigit == 0)
		{
			pAnita(i)->SetState(STATE_VISIBLE, false);   // leading zero
		}
		else
		{
			pAnita(i)->SetAnimNo(ScoreAnm(nDigit, nKind), false);
			pAnita(i)->SetState(STATE_VISIBLE, true);
		}
	}
}

// DOL 0x800c0d84
void cTaskConsMultiMenuPlayer::WaitExec()
{
	SetScoreAnm(1);
	pAnita(3u)->SetAnimNo(MARK_ANM[1], true);   // UNVERIFIED: reset = 1 every frame as in the DOL (r5 = 1)
	pAnita(3u)->SetState(STATE_VISIBLE, m_bMark);
}

// DOL 0x800c0e18
bool cTaskConsMultiMenuPlayer::AppearExec()
{
	if (nStep() == 0)
	{
		if (!pAnita(1u)->IsState(STATE_VISIBLE))
		{
			pAnita(1u)->SetAnimNo(FRAME1_ANM[m_nPlayer][0], true);
			pAnita(1u)->SetState(STATE_VISIBLE, true);
		}
		if (!pAnita(2u)->IsState(STATE_VISIBLE))
		{
			pAnita(2u)->SetAnimNo(FRAME2_ANM[m_nPlayer][0], true);
			pAnita(2u)->SetState(STATE_VISIBLE, true);
		}
		pAnita(3u)->SetAnimNo(MARK_ANM[0], true);
		pAnita(3u)->SetState(STATE_VISIBLE, m_bMark);
		SetScoreAnm(0);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool bEnd = pAnita(1u)->IsState(STATE_ANM_END) && pAnita(2u)->IsState(STATE_ANM_END);
		if (bEnd)
		{
			for (uint i = 4u; i <= 9u; i++)
			{
				if (pAnita(i)->IsState(STATE_VISIBLE) && !pAnita(i)->IsState(STATE_ANM_END))
				{
					bEnd = false;
				}
				if (!bEnd)
				{
					break;
				}
			}
		}
		if (bEnd)
		{
			pAnita(1u)->SetAnimNo(FRAME1_ANM[m_nPlayer][1], true);
			pAnita(2u)->SetAnimNo(FRAME2_ANM[m_nPlayer][1], true);
			if (m_bMark)
			{
				pAnita(3u)->SetAnimNo(MARK_ANM[1], true);
			}
			SetScoreAnm(1);
			if (!m_bFontVisible)
			{
				m_bFontVisible = true;   // cTaskFontWriter +0x268
			}
			m_bFaceVisible = true;       // face icon(0) +0x7c
			return true;
		}
	}
	return false;
}

// DOL 0x800c1268 (the frames, name and face are left as they are)
bool cTaskConsMultiMenuPlayer::DeleteExec()
{
	if (nStep() == 0)
	{
		if (m_bMark)
		{
			pAnita(3u)->SetAnimNo(MARK_ANM[2], true);
		}
		SetScoreAnm(2);
		nStep()++;
	}
	else if (nStep() == 1)
	{
		bool bEnd = true;
		for (uint i = 4u; bEnd && i <= 9u; i++)
		{
			if (pAnita(i)->IsState(STATE_VISIBLE) && !pAnita(i)->IsState(STATE_ANM_END))
			{
				bEnd = false;
			}
		}
		if (bEnd)
		{
			return true;
		}
	}
	return false;
}

// DOL 0x800c1368
bool cTaskConsMultiMenuPlayer::Execute(cJetTask* pParam)
{
	return cTaskConsMenuBase::Execute(pParam);   // DOL 0x80145740; the name print below runs in Draw (the writer child draws it natively)
}

// The cTaskFontWriter child (vtable 0x804381c0, priority 0x80000002) prints the name at ((int)x, (int)-y) once its visible byte (+0x268) is set
// (Execute 0x800c1368 queues it with 0x80016e88, the name as the format). UNVERIFIED: the font (KingSys +0xa68 is taken as RESIDENT_FONT_MII, the
// Wii name font), the size 18 (0x80016dd8) and the baseline origin of the writer's default flags.
void cTaskConsMultiMenuPlayer::Draw(int nStatus, cJetTask* pParam)
{
	cTaskConsMenuBase::Draw(nStatus, pParam);
	if (!m_bFontVisible || m_pName == nullptr || m_pName[0] == '\0') return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";
	const int h = gfx::FontHeight(st.font);
	if (h > 0) st.scaleX = st.scaleY = 18.0f / (float)h;
	st.color = 0xffffffffu;
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	gfx::DrawText((float)(int)m_vFontPos.x, (float)(int)-m_vFontPos.y, m_pName, st);
}

}
