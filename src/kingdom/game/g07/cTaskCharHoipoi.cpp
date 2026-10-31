#include "kingdom/game/g07/cTaskCharHoipoi.hpp"

#include "cJetTask.hpp"
#include "kingdom/cKingdomModel.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

cTaskCharHoipoi::cTaskCharHoipoi()
{
	Init();
}

Vector4& cTaskCharHoipoi::vTarget()
{
	return m_vTarget;
}

// DOL 0x800ad628 [audited: per AUDIT_G03_G07 row]: Create(0), rotate unit, scale (1, 1, 1)
void cTaskCharHoipoi::Init()
{
	Create(0u);
	::Constants::jetGeoVecUnit(vRotate());
	::Constants::jetGeoVecUnit(vScale());
	Vector4& reference = vScale();
	Vector4& reference2 = vScale();
	float num = (vScale()[2] = 1.0f);
	float value = (reference2[1] = num);
	reference[0] = value;
}

void cTaskCharHoipoi::Term()
{
	cTaskChar::Term();
}

// DOL 0x800ad6d0 [audited: per AUDIT_G03_G07 row]
bool cTaskCharHoipoi::Create(uint no)
{
	cKingdomModel* cKingdomModel2 = CreateModel();
	cKingdomModel2->RegistModel("MDL_CHAR_PACMAN_CAPSULE");
	string array[2] = {"TSA_CHAR_PACMAN_CAPSULE", "CLA_CHAR_PACMAN_CAPSULE"};
	int num = ::Constants::ARRAY_SIZE(array);
	for (int i = 0; i < num; i++)
	{
		cKingdomModel2->RegistAnm(0u, array[i]);
	}
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	SaveFrameRate();
	StopFrameRate();
	cKingdomModel2->Invisible();
	return true;
}

// DOL 0x800ad8d4 [audited: per AUDIT_G03_G07 row]: model trans / rotate / scale <- +0x1e0 / +0x200 / scale
void cTaskCharHoipoi::UpdateMatrix()
{
	cKingdomModel* cKingdomModel2;
	if ((cKingdomModel2 = pModel()) != nullptr)
	{
		::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), vMTrans());
		::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
		::Constants::jetGeoVecCopy(cKingdomModel2->vScale(), vScale());
	}
}

// DOL 0x800ad980 [audited: per AUDIT_G03_G07 row]: ActionLocal (+0xa0), Move (+0x94), UpdateFrame 0x801e3234, UpdateMatrix (+0x50)
bool cTaskCharHoipoi::Execute(cJetTask* pParam)
{
	ActionLocal();
	Move();
	UpdateFrame();
	UpdateMatrix();
	return true;
}

// DOL 0x800ad9ec [audited: per AUDIT_G03_G07 row]
bool cTaskCharHoipoi::AnimateModel(uint model)
{
	float frameAnmTexSrt = pModel(model)->GetFrameAnmTexSrt(0u);
	float frameMatClr = pModel(model)->GetFrameMatClr(0u);
	if (frameAnmTexSrt >= pModel(model)->GetFrameMaxAnmTexSrt(0u) && frameMatClr >= pModel(model)->GetFrameMaxMatClr(0u))
	{
		return true;
	}
	return false;
}

void cTaskCharHoipoi::InitFrame()
{
	if (pModel() != nullptr)
	{
		pModel()->Visible();
		pModel()->SetFrameAnmTexSrt(0u, 0.0f);
		pModel()->SetFrameMatClr(0u, 0.0f);
	}
}

void cTaskCharHoipoi::LoadFrameRate()
{
	if (pModel() != nullptr)
	{
		pModel()->SetUpdateRateAnmTexSrt(0u, m_fSpeed[0]);
		pModel()->SetUpdateRateMatClr(0u, m_fSpeed[1]);
	}
}

void cTaskCharHoipoi::StopFrameRate()
{
	if (pModel() != nullptr)
	{
		LoadFrameRate();
		pModel()->Invisible();
		pModel()->SetUpdateRateAnmTexSrt(0u, 0.0f);
		pModel()->SetUpdateRateMatClr(0u, 0.0f);
	}
}

void cTaskCharHoipoi::SaveFrameRate()
{
	if (pModel() != nullptr)
	{
		m_fSpeed[0] = pModel()->GetUpdateRateAnmTexSrt(0u);
		m_fSpeed[1] = pModel()->GetUpdateRateMatClr(0u);
	}
}

bool cTaskCharHoipoi::Move()
{
	return true;
}

bool cTaskCharHoipoi::ActionLocal()
{
	return true;
}

bool cTaskCharHoipoi::ActionOther()
{
	return true;
}

}
