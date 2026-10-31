#include "kingdom/game/g07/cTaskCharItem.hpp"

#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

const MO_CHAR_PARAMS* cTaskCharItem::g_MoItemParam[4] = {[]() { MO_CHAR_PARAMS* _o1 = new MO_CHAR_PARAMS(); _o1->m_fWeight = 4.0f; _o1->m_nHereticVal = 2u; _o1->m_fMaxSpeed = 0.15f; _o1->m_fAccelSpeed = 0.05f; _o1->m_fDecelSpeed = 0.75f; _o1->m_fHitPower = 1.0f; _o1->m_fChargePower = 0.5f; _o1->m_fSlopePower = 0.05f; _o1->m_fWallReflect = 1.5f; _o1->m_fHitSize = 0.48f; _o1->m_fWHitSize = 0.4f; _o1->m_nPoints = 100; _o1->m_nVSPoints = 10; return _o1; }(), []() { MO_CHAR_PARAMS* _o2 = new MO_CHAR_PARAMS(); _o2->m_fWeight = 4.0f; _o2->m_nHereticVal = 2u; _o2->m_fMaxSpeed = 0.15f; _o2->m_fAccelSpeed = 0.05f; _o2->m_fDecelSpeed = 0.75f; _o2->m_fHitPower = 1.0f; _o2->m_fChargePower = 0.5f; _o2->m_fSlopePower = 0.05f; _o2->m_fWallReflect = 1.5f; _o2->m_fHitSize = 0.48f; _o2->m_fWHitSize = 0.4f; _o2->m_nPoints = 100; _o2->m_nVSPoints = 10; return _o2; }(), []() { MO_CHAR_PARAMS* _o3 = new MO_CHAR_PARAMS(); _o3->m_fWeight = 4.0f; _o3->m_nHereticVal = 2u; _o3->m_fMaxSpeed = 0.15f; _o3->m_fAccelSpeed = 0.05f; _o3->m_fDecelSpeed = 0.75f; _o3->m_fHitPower = 1.0f; _o3->m_fChargePower = 0.5f; _o3->m_fSlopePower = 0.05f; _o3->m_fWallReflect = 1.5f; _o3->m_fHitSize = 0.48f; _o3->m_fWHitSize = 0.4f; _o3->m_nPoints = 100; _o3->m_nVSPoints = 10; return _o3; }(), []() { MO_CHAR_PARAMS* _o4 = new MO_CHAR_PARAMS(); _o4->m_fWeight = 4.0f; _o4->m_nHereticVal = 2u; _o4->m_fMaxSpeed = 0.15f; _o4->m_fAccelSpeed = 0.05f; _o4->m_fDecelSpeed = 0.75f; _o4->m_fHitPower = 1.0f; _o4->m_fChargePower = 0.5f; _o4->m_fSlopePower = 0.05f; _o4->m_fWallReflect = 1.5f; _o4->m_fHitSize = 0.48f; _o4->m_fWHitSize = 0.4f; _o4->m_nPoints = 100; _o4->m_nVSPoints = 10; return _o4; }()};

cTaskCharItem::cTaskCharItem()
	
{
	Init();
}

bool cTaskCharItem::Create(uint no)
{
	return false;
}

// DOL 0x801f190c [audited: per AUDIT_G03_G07 row]
void cTaskCharItem::Init()
{
	OffFlag(4u);
	OnFlag(16u);
}

void cTaskCharItem::Term()
{
	cTaskCharEnemy::Term();
}

// DOL 0x801f1924 [audited: per AUDIT_G03_G07 row]
bool cTaskCharItem::ActionBoth()
{
	if (IsFall())
	{
		ChangeFallMove();
	}
	return true;
}

// DOL 0x801f1980 [audited: per AUDIT_G03_G07 row]
bool cTaskCharItem::ActionLocal()
{
	nInputDir() = (ushort)Input();
	Dir2Rotate(nModelDir());
	return true;
}

// DOL 0x801f19d8 [audited: per AUDIT_G03_G07 row]
bool cTaskCharItem::ActionOther()
{
	Dir2Rotate(nModelDir());
	return true;
}

void cTaskCharItem::Setting(int kind)
{
	nKind() = (uint)kind;
	InitParam(g_MoItemParam[kind]);
}

}
