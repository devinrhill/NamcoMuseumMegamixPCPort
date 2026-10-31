#include "kingdom/game/g03/cTaskLight.hpp"

#include "cJetTask.hpp"
#include "kingdom/NativeScene.hpp"

namespace kingdom::game::g03
{

// DOL 0x80185b10 [audited: per AUDIT_G03_G07 row] (eTYPE_FOLLOW; the offset is the light's position in the SCN0)
cTaskLight::cTaskLight(int set, int rig, const Vector4* target)
{
	SetName("cTaskLight");
	Init();
	m_nType = eTYPE_FOLLOW;
	m_nSet = set;
	m_nRig = rig;
	m_pTarget = target;
	const wii::LightResult* light = kingdom::NativeScene::Light(set);
	m_vOffset = Vector4(light->rig[rig].pos[0], light->rig[rig].pos[1], light->rig[rig].pos[2], 1.0f);
}

// DOL 0x80185c7c [audited: class row done in the 43rd session; function not individually re-read]
cTaskLight::~cTaskLight()
{
	Term();
}

// DOL 0x80185d00 [audited: described by the 43rd-session pass] (+0x1b4 / +0x1b8 / +0x1bc = 0: type, light object, node pointer)
void cTaskLight::Init()
{
	m_nType = 0u;
}

void cTaskLight::Term()
{
}

bool cTaskLight::Execute(cJetTask* pParam)
{
	return true;
}

// DOL 0x80185d20 [audited: per AUDIT_G03_G07 row]: LightObj::InitLightPos(target + offset); only the position moves, the aim / specular direction stay as authored
void cTaskLight::Draw(int nStatus, cJetTask* pParam)
{
	if (m_pTarget == nullptr)
	{
		return;
	}
	wii::LightResult light = *kingdom::NativeScene::Light(m_nSet);
	light.rig[m_nRig].pos[0] = m_pTarget->x + m_vOffset.x;
	light.rig[m_nRig].pos[1] = m_pTarget->y + m_vOffset.y;
	light.rig[m_nRig].pos[2] = m_pTarget->z + m_vOffset.z;
	kingdom::NativeScene::SetLight(m_nSet, light);
}

}
