#include "kingdom/game/g02/cTaskMenuMap.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "kingdom/game/g02/cTaskMenuCarBase.hpp"

// Rally-X Remix (g02) stage select world map, ported 1:1 from main.dol (no C# exists for Rally-X).

namespace kingdom::game::g02
{

namespace
{
constexpr float DEG2RAD = 0.01745329238474369f;   // FLOAT_8047bb00
constexpr float APPEAR_STEP = 0.075f;              // FLOAT_8047bb18
constexpr float PULSE_LOW = 0.9f;                  // FLOAT_8047bb20
constexpr float PULSE_HIGH = 1.1f;                 // FLOAT_8047bb24
constexpr float POINT_SPEED = 0.01f;               // FLOAT_8047bb1c / -FLOAT_8047bb28
constexpr float DECIDE_SPEED = 0.02f;              // FLOAT_8047bb2c / -FLOAT_8047bb30

// DOL 0x80377cb8 [audited]: the model of stage index n (stage + world * 6); stages 4..6 of a world reuse its three maps
const char* const WORLD_MODEL[24] = {
	"MENU_WORLD_1A", "MENU_WORLD_1B", "MENU_WORLD_1C", "MENU_WORLD_1A", "MENU_WORLD_1B", "MENU_WORLD_1C",
	"MENU_WORLD_2A", "MENU_WORLD_2B", "MENU_WORLD_2C", "MENU_WORLD_2A", "MENU_WORLD_2C", "MENU_WORLD_2B",
	"MENU_WORLD_3A", "MENU_WORLD_3B", "MENU_WORLD_3C", "MENU_WORLD_3B", "MENU_WORLD_3C", "MENU_WORLD_3A",
	"MENU_WORLD_4A", "MENU_WORLD_4B", "MENU_WORLD_4C", "MENU_WORLD_4B", "MENU_WORLD_4C", "MENU_WORLD_4A",
};
}

// DOL 0x80111d84 [audited]: the cTaskMenuBgBase part (+0x1d8.. = 0, +0x1e0 = 0, the cTaskModel child +0x1e8), then priority 0x10003, world / stage,
// the shown model = stage + world * 6, pulse step 0
cTaskMenuMap::cTaskMenuMap(kingdom::game::cTaskConsMenuManBase* const& pConsMan, int nWorldNo, int nStageNo)
	: cTaskConsMenuBase(pConsMan)
{
	SetName("g02::cTaskMenuMap");
	m_nBgState = 0;
	m_nBgStep = 0;
	m_fScale = 0.0f;
	m_nCurIdx = 0;
	m_pModel = new kingdom::cTaskModel();
	ConnectChild(m_pModel);
	nDrawPriority(0x10003u);
	m_nWorldNo = nWorldNo;
	m_nStageNo = nStageNo;
	m_nCurIdx = nStageNo + nWorldNo * 6;
	m_fSpeed = 0.0f;
}

// DOL 0x801127b8 [audited]
cTaskMenuMap::~cTaskMenuMap()
{
}

// DOL 0x80111ec8 [audited]: model 24 = MENU_NULL_MODEL; models 0..23 = WORLD_MODEL[n] + MENU_WORLD_ANIM (crc 0xbc72f62d), light set 0, hidden, each placed
// at node "set_bg" of the null model (rotation degrees -> radians); then the hit layout on anita 0
void cTaskMenuMap::RegistData()
{
	cKingdomModel* null = m_pModel->CreateModel(0x18u);
	null->RegistModel("MENU_NULL_MODEL", 0x36fu);
	for (uint n = 0; n < 0x18u; n++)
	{
		cKingdomModel* m = m_pModel->CreateModel(n);
		m->RegistModel(WORLD_MODEL[n], 0x36fu);
		m->RegistAnm(0u, "MENU_WORLD_ANIM");   // FUN_8001a4d8(m, 0, crc, 0, 1)
		m->SetLightSetIdx(0);
		m->Invisible();
		m_vTrans = Vector4(0.0f, 0.0f, 0.0f, 1.0f);   // the stack defaults of the DOL (0, 0, 0, 1) / ONE
		m_vRotate = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		m_vScale = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		MenuSetNode("MENU_NULL_MODEL", "set_bg", m_vTrans, m_vRotate, m_vScale);   // s_set_bg_80377dac
		m_vRotate = Vector4(DEG2RAD * m_vRotate.x, DEG2RAD * m_vRotate.y, DEG2RAD * m_vRotate.z, m_vRotate.w);
		m->vTrans() = m_vTrans;
		m->vRotate() = m_vRotate;
		m->vScale() = m_vScale;
	}
	pAnita(0u)->RegistData(ResourceDatabase::GetPath("ANITA_MENU_SELECT_01"));
}

// DOL 0x801089f4 [audited]
void cTaskMenuMap::ChangeData(int nIdx)
{
	m_pModel->pModel((uint)m_nCurIdx)->Invisible();
	m_nCurIdx = nIdx;
	m_pModel->pModel((uint)m_nCurIdx)->Visible();
}

// DOL 0x80112114 [audited]
bool cTaskMenuMap::IsDpdHit(float x, float y)
{
	return DpdHitPat(x, y, 0x24);
}

void cTaskMenuMap::ApplyScale()
{
	cKingdomModel* m = m_pModel->pModel((uint)m_nCurIdx);
	m->vScale() = Vector4(m_vScale.x * m_fScale, m_vScale.y * m_fScale, m_vScale.z * m_fScale, m->vScale().w);
}

// DOL 0x8011224c [audited]: (bg step 0 only) full size, the pulse stops
void cTaskMenuMap::WaitExec()
{
	if (m_nBgStep == 0)
	{
		m_fSpeed = 0.0f;
		m_fScale = 1.0f;
		ApplyScale();
	}
}

// DOL 0x8011230c [audited]: the model shows and grows from 0 to full size by 0.075 a frame
bool cTaskMenuMap::AppearExec()
{
	if (m_nBgStep == 0)
	{
		m_fScale = 0.0f;
		m_pModel->pModel((uint)m_nCurIdx)->Visible();
		m_nBgStep++;
	}
	else if (m_nBgStep == 1)
	{
		m_fScale += APPEAR_STEP;
		if (1.0f <= m_fScale)
		{
			m_fScale = 1.0f;
			m_nBgState = 2;
			m_nBgStep = 0;
			return true;   // the DOL returns before the scale store of this frame
		}
	}
	ApplyScale();
	return false;
}

// DOL 0x80112430 [audited]: the pointed map pulses between 0.9 and 1.1 (+-0.01 a frame)
void cTaskMenuMap::PointExec()
{
	if (m_fSpeed == 0.0f) m_fSpeed = POINT_SPEED;
	if (m_fScale < PULSE_LOW)
		m_fSpeed = POINT_SPEED;
	else if (PULSE_HIGH < m_fScale)
		m_fSpeed = -POINT_SPEED;
	m_fScale += m_fSpeed;
	ApplyScale();
}

// DOL 0x8011252c [audited]: the same pulse at +-0.02; never ends (the man waits for the stage button, not for the map)
bool cTaskMenuMap::DecideExec()
{
	if (m_fSpeed == 0.0f) m_fSpeed = DECIDE_SPEED;
	if (m_fScale < PULSE_LOW)
		m_fSpeed = DECIDE_SPEED;
	else if (PULSE_HIGH < m_fScale)
		m_fSpeed = -DECIDE_SPEED;
	m_fScale += m_fSpeed;
	ApplyScale();
	return false;
}

// DOL 0x8011262c [audited]: shrinks from 1 to 0 by 0.075 a frame, then hides
bool cTaskMenuMap::DeleteExec()
{
	if (m_nBgStep == 0)
	{
		m_fScale = 1.0f;
		m_nBgStep = 1;
	}
	else if (m_nBgStep == 1)
	{
		m_fScale -= APPEAR_STEP;
		if (m_fScale <= 0.0f)
		{
			m_fScale = 0.0f;
			m_pModel->pModel((uint)m_nCurIdx)->Invisible();
			m_nBgState = 4;
			m_nBgStep = 0;
			return true;
		}
	}
	ApplyScale();
	return false;
}

// DOL 0x8011274c [audited]: the console state machine (cTaskConsMenuBase::Execute 0x80145740), then the model clips step (0x8009a478)
bool cTaskMenuMap::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	const bool r = cTaskConsMenuBase::Execute(pParam);
	m_pModel->UpdateFrame();
	return r;
}

}
