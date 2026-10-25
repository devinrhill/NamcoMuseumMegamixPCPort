#include <cstdlib>
#include <cstdio>
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "DummyNW4R.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "ResMdl.hpp"
#include "ResNode.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g03
{

uint cTaskBgBox::nKind()
{
	return m_nKind;
}

bool cTaskBgBox::IsBreak()
{
	return m_bBreak;
}

uint cTaskBgBox::nTreasure()
{
	return m_nTreasure;
}

bool cTaskBgBox::IsBreakEnd()
{
	return m_bBreakEnd;
}

cTaskBgBox::cTaskBgBox()
{
	SetName("cTaskBgBox");
	Init();
}

void cTaskBgBox::Init()
{
	m_nKind = 0u;
	m_nTreasure = 0u;
	m_bBreak = false;
	this->Model->Visible();
}

void cTaskBgBox::Term()
{
	cTaskBgHit::Term();
}

bool cTaskBgBox::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	if (Constants::IS_PAC_PAUSE())
	{
		return true;
	}
	// DOL 0x80156528: while the box is broken its model animation (BIN_BOX_*_VIA, a VIS0 clip that hides the box node and shows the debris) runs one frame per
	// Execute; nothing else is done here. (The C# sampled the clip by hand with a Unity Animation and toggled m_viaAnimTarget.)
	if (IsBreak() && !IsBreakEnd())
	{
		this->Model->UpdateFrame();
		const float max = this->Model->GetFrameMaxAnmVis(0u);
		if (max < 0.0f || this->Model->GetFrameAnmVis(0u) >= max)
		{
			m_bBreakEnd = true;
			if (this->Model->IsVisible()) this->Model->Invisible();
		}
	}
	UpdateMatrix();
	if (pFaceList() != nullptr && !IsBreak())
	{
		cAppG03PacnRollGame2->pTaskCollision()->PushBackCollisionObj(this);
	}
	return true;
}

void cTaskBgBox::Draw(int nStatus, cJetTask* pParam)
{
	Matrix4x4 matrix = pMtxRotTrans();
	DummyNW4R::SetMtx(this->Model->pScnMdl(), matrix);
	DummyNW4R::SetScale(this->Model->pScnMdl(), this->Model->vScale()[0], this->Model->vScale()[1], this->Model->vScale()[2]);
	cTaskBgHit::Draw(nStatus, pParam);
}

bool cTaskBgBox::RegistData(const uint& kind, const uint& treasure, const Vector4& trans, const Vector4& rotate, const bool& isBreak)
{
	CustomKingdomUtil::Assert(4 > kind, "error");
	CustomKingdomUtil::Assert(23 > treasure, "error");
	CustomKingdomUtil::Assert("nullpo");
	CustomKingdomUtil::Assert("nullpo");
	m_nKind = kind;
	m_bBreak = isBreak;
	string text;
	string text2;
	string text3;
	switch (kind)
	{
	case 0u:
		text = "BIN_BOX_WOOD_L_MDL";
		text2 = "BIN_BOX_L_HIT_MDL";
		text3 = "BIN_BOX_WOOD_L_VIA";
		SetState(1u);
		break;
	case 1u:
		text = "BIN_BOX_WOOD_S_MDL";
		text2 = "BIN_BOX_S_HIT_MDL";
		text3 = "BIN_BOX_WOOD_S_VIA";
		SetState(1u);
		break;
	case 2u:
		text = "BIN_BOX_IRON_L_MDL";
		text2 = "BIN_BOX_L_HIT_MDL";
		text3 = "BIN_BOX_IRON_L_VIA";
		break;
	case 3u:
		text = "BIN_BOX_IRON_S_MDL";
		text2 = "BIN_BOX_S_HIT_MDL";
		text3 = "BIN_BOX_IRON_S_VIA";
		break;
	default:
		CustomKingdomUtil::Assert("nullpo");
		return false;
	}
	this->Model->RegistModel(text);
	this->Model->RegistAnm(0u, text3);
	RegistHitData(text2);
	this->Model->SetLightSetIdx(3);
	this->Model->SetFogIdx(0);
	::Constants::jetGeoVecCopy(this->Model->vTrans(), trans);
	::Constants::jetGeoVecCopy(this->Model->vRotate(), rotate);
	this->Model->AttachShadow();
	if (!isBreak)
	{
		m_nTreasure = treasure;
		Constants::PACN_APP()->pTaskItem()->SetBoxItem(this->Model->vTrans(), this->Model->vRotate()[1], treasure, this);
		this->Model->Visible();
	}
	else
	{
		this->Model->Invisible();
		m_bBreakEnd = true;
	}
	ResMdl* resMdl = new ResMdl(this->Model->pScnMdl());
	ResNode* resNode = nullptr;
	switch (kind)
	{
	case 0u:
		resNode = resMdl->GetResNode("wbox_l");
		break;
	case 1u:
		resNode = resMdl->GetResNode("wbox_s");
		break;
	case 2u:
		resNode = resMdl->GetResNode("ibox_l");
		break;
	case 3u:
		resNode = resMdl->GetResNode("ibox_s");
		break;
	}
	if (resNode != nullptr && resNode->IsValid())
	{
		// TODO(unity): m_viaAnimTarget = resNode.This.gameObject;
	}
	return true;
}

bool cTaskBgBox::RegistData(sSaveDataBox& box)
{
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	v[1] = ::Constants::JET_DEG2RAD(box.rotY);
	if (std::getenv("MEGAMIX_GATELOG")) std::fprintf(stderr, "box: kind %d treasure %d trans (%.2f %.2f %.2f) rotY %.1f\n", (int)box.kind, (int)box.treasure, box.trans[0], box.trans[1], box.trans[2], (double)box.rotY);
	return RegistData(box.kind, box.treasure, box.trans, v, false);
}

void cTaskBgBox::HitCallbackWall(Constants::sFace* face, Vector4 hitPosL, Vector4 hitPosW)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	if (cTaskCharPacman2->IsState(1u))
	{
		if (m_nKind == 0 || m_nKind == 1)
		{
			m_bBreak = true;
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE037_BOX_HAKAI);
		}
		else if (cTaskCharPacman2->nType() == 2)
		{
			m_bBreak = true;
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE037_BOX_HAKAI);
		}
		else
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE035_METAL_BOX, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
		}
	}
	else if (m_nKind != 0)
	{
		(void)(m_nKind);
		(void)(1);
	}
}

}
