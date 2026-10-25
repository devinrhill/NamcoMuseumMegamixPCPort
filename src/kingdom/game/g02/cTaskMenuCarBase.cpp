#include "kingdom/game/g02/cTaskMenuCarBase.hpp"

#include "KingdomGlobal.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "wii/Archive.hpp"
#include "wii/Brres.hpp"
#include "wii/Mdl0.hpp"

// Rally-X Remix (g02) stage select: Pac-Man's car, ported 1:1 from main.dol (no C# exists for Rally-X).

namespace kingdom::game::g02
{

namespace
{
constexpr float DEG2RAD = 0.01745329238474369f;   // FLOAT_8047bae0
constexpr float APPEAR_STEP = 0.075f;              // FLOAT_8047bae4
}

// FUN_8001dd54 / FUN_8001dbd4 / FUN_8001da54 read the ResNodeData of the named node (+0x38 translate, +0x2c rotate, +0x20 scale; w =
// FLOAT_80479e64, natively 1). Natively the resource is parsed again here (the port's cKingdomModel keeps no ResNode accessors).
bool MenuSetNode(const char* model, const char* node, Vector4& trans, Vector4& rot, Vector4& scale)
{
	std::size_t size = 0;
	const u8* d = wii::ArchiveStore::Find(model, size);
	wii::Brres brres;
	wii::Mdl0 mdl;
	if (d == nullptr || !brres.Load(d, size) || brres.ModelNames().empty() || !mdl.Load(brres, brres.ModelNames()[0])) return false;
	for (const wii::Mdl0Node& n : mdl.nodes)
	{
		if (n.name != node) continue;
		trans = Vector4(n.translate[0], n.translate[1], n.translate[2], 1.0f);
		rot = Vector4(n.rotate[0], n.rotate[1], n.rotate[2], 1.0f);
		scale = Vector4(n.scale[0], n.scale[1], n.scale[2], 1.0f);
		return true;
	}
	return false;
}

cTaskMenuCarBase::cTaskMenuCarBase()
{
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x80110c54
cTaskMenuCarBase::~cTaskMenuCarBase()
{
}

// DOL 0x80110bac
cTaskMenuPacman::cTaskMenuPacman()
{
	SetName("g02::cTaskMenuPacman");
	nDrawPriority(0x10004u);
	m_fScale = 0.0f;
}

// DOL 0x80111148
cTaskMenuPacman::~cTaskMenuPacman()
{
}

// DOL 0x80110cac: model 1 = MENU_NULL_MODEL (0x72dd0c3c, the placement); model 0 = CAR_PAC (0x973daad2) + CAR_PAC_ANIM_NORMAL, light set 1,
// shown; its transform = node "set_pac" of the null model (rotation degrees -> radians), scale 0 until it appears. BODY_PAC on the car's node
// "man_root" (light set 1 is set on the car again, not on the body), HEAD_PAC on the body's node "face" + HEAD_PAC_ANIM_NORMAL; the body
// gets light set 1, the head keeps its resource light set (the DOL sets none on it).
void cTaskMenuPacman::Init()
{
	cKingdomModel* null = CreateModel(1u);
	null->RegistModel("MENU_NULL_MODEL", 0x36fu);
	cKingdomModel* car = CreateModel(0u);
	car->RegistModel("CAR_PAC", 0x36fu);
	car->RegistAnm(0u, "CAR_PAC_ANIM_NORMAL");   // FUN_8001a4d8(model, 0, crc 0x47a86811, 0, 1)
	car->SetLightSetIdx(1);
	car->Visible();
	m_vTrans = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	m_vRotate = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	m_vScale = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	MenuSetNode("MENU_NULL_MODEL", "set_pac", m_vTrans, m_vRotate, m_vScale);   // PTR s_set_pac_80377c94
	m_vRotate = Vector4(DEG2RAD * m_vRotate.x, DEG2RAD * m_vRotate.y, DEG2RAD * m_vRotate.z, m_vRotate.w);
	car->vTrans() = m_vTrans;
	car->vRotate() = m_vRotate;
	car->vScale() = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	pModel(0u)->RegistExpand(0u, "man_root", "BODY_PAC", 0x36fu);
	pModel(0u)->SetLightSetIdx(1);
	cKingdomModel* body = pModel(0u)->pExpandMdl(0u);
	if (body != nullptr)
	{
		body->RegistExpand(0u, "face", "HEAD_PAC", 0x36fu);
		if (cKingdomModel* head = body->pExpandMdl(0u)) head->RegistAnm(0u, "HEAD_PAC_ANIM_NORMAL");   // crc 0x17ef6931
		body->SetLightSetIdx(1);
	}
}

// DOL 0x80110b74
void cTaskMenuPacman::AppearState()
{
	m_nState = 1;
	m_nStep = 0;
}

// DOL 0x80110b88
bool cTaskMenuPacman::IsAppearEnd()
{
	return m_nState == 2;
}

// DOL 0x80110b9c
void cTaskMenuPacman::Reset()
{
	m_nState = 0;
	m_nStep = 0;
}

// DOL 0x80110ef4: unless paused: appearing = the car's scale grows from 0 to its node scale by 0.075 a frame; then the clips step
// (cTaskModel::UpdateFrame 0x8009a478) and the model task runs
bool cTaskMenuPacman::Execute(cJetTask* pParam)
{
	if (KingdomGlobal::KingSys->pAppMain()->IsPause()) return true;
	if (m_nState == 1)
	{
		if (m_nStep == 0)
		{
			m_fScale = 0.0f;
			m_nStep = 1;
		}
		else if (m_nStep == 1)
		{
			m_fScale += APPEAR_STEP;
			if (1.0f <= m_fScale)
			{
				m_fScale = 1.0f;
				m_nState = 2;
				m_nStep = 0;
			}
		}
		const float f = m_fScale;
		cKingdomModel* car = pModel(0u);
		car->vScale() = Vector4(m_vScale.x * f, m_vScale.y * f, m_vScale.z * f, car->vScale().w);
	}
	UpdateFrame();
	cTaskModel::Execute(pParam);
	return true;
}

}
