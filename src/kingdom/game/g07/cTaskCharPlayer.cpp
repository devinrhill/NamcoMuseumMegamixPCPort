#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/cTaskMap.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomModel.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"
#include "kingdom/game/g07/cTaskCharBoss.hpp"
#include "kingdom/game/g07/cTaskCharHoipoi.hpp"
#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <SoundManager.hpp>
#include <SoundSe.hpp>
#include <SoundSeObject.hpp>

namespace kingdom::game::g07
{

const cTaskCharPlayer::sANIMATION_LIST* cTaskCharPlayer::AnimationName[4] = {[]() { cTaskCharPlayer::sANIMATION_LIST* _o1 = new cTaskCharPlayer::sANIMATION_LIST(); _o1->anm[0] = "CHA_CHAR_PACMAN_01_STANDARD"; _o1->anm[1] = "<nullpo>"; _o1->anm[2] = "<nullpo>"; _o1->anm[3] = "<nullpo>"; _o1->anm[4] = "CHA_CHAR_PACMAN_01_STANDARD"; return _o1; }(), []() { cTaskCharPlayer::sANIMATION_LIST* _o2 = new cTaskCharPlayer::sANIMATION_LIST(); _o2->anm[0] = "CHA_CHAR_PACMAN_01_FORCE"; _o2->anm[1] = "<nullpo>"; _o2->anm[2] = "<nullpo>"; _o2->anm[3] = "<nullpo>"; _o2->anm[4] = "CHA_CHAR_PACMAN_01_STANDARD"; return _o2; }(), []() { cTaskCharPlayer::sANIMATION_LIST* _o3 = new cTaskCharPlayer::sANIMATION_LIST(); _o3->anm[0] = "CHA_CHAR_PACMAN_01_MOBILITY"; _o3->anm[1] = "CHA_CHAR_PACMAN_11_MOBILITY"; _o3->anm[2] = "CHA_CHAR_PACMAN_12_MOBILITY"; _o3->anm[3] = "CHA_CHAR_PACMAN_13_MOBILITY"; _o3->anm[4] = "CHA_CHAR_PACMAN_01_STANDARD"; return _o3; }(), []() { cTaskCharPlayer::sANIMATION_LIST* _o4 = new cTaskCharPlayer::sANIMATION_LIST(); _o4->anm[0] = "CHA_CHAR_PACMAN_01_MASTER"; _o4->anm[1] = "CHA_CHAR_PACMAN_11_MASTER"; _o4->anm[2] = "CHA_CHAR_PACMAN_12_MASTER"; _o4->anm[3] = "CHA_CHAR_PACMAN_13_MASTER"; _o4->anm[4] = "CHA_CHAR_PACMAN_01_STANDARD"; return _o4; }()};

cTaskCharPlayer::cTaskCharPlayer()
{
	Init();
}

cTaskCharPlayer::cTaskCharPlayer(const string& key, uint option)
	: cTaskChar(key, option)
{
	Init();
}

cTaskCharPlayer::cTaskCharPlayer(Object* data, uint option)
	: cTaskChar(data, option)
{
	Init();
}

bool& cTaskCharPlayer::bPlayerMoveStop()
{
	return m_bPlayerMoveStop;
}

Constants::MO_PLAYER_PARAMS*& cTaskCharPlayer::pPLParam()
{
	return m_PLParam;
}

float& cTaskCharPlayer::fMaxSpeedHover()
{
	return m_PLParam->m_fMaxSpeedHover;
}

float& cTaskCharPlayer::fAccelSpeedHover()
{
	return m_PLParam->m_fAccelSpeedHover;
}

float& cTaskCharPlayer::fDecelSpeedHover()
{
	return m_PLParam->m_fDecelSpeedHover;
}

uint& cTaskCharPlayer::nHoverTimer()
{
	return m_nHoverTimer;
}

int& cTaskCharPlayer::nPowerLevel()
{
	return m_nPowerLevel;
}

int& cTaskCharPlayer::nHoverLevel()
{
	return m_nHoverLevel;
}

int& cTaskCharPlayer::nChargeLevel()
{
	return m_nChargeLevel;
}

int& cTaskCharPlayer::nChargePowLevel()
{
	return m_nChargePowLevel;
}

int& cTaskCharPlayer::nChargePowLevelOLD()
{
	return m_nChargePowLevelOLD;
}

int& cTaskCharPlayer::nChargeCnt()
{
	return m_nChargeCnt;
}

int& cTaskCharPlayer::nFunbariMeter()
{
	return m_nFunbariMeter;
}

int& cTaskCharPlayer::nFunbariHeretic()
{
	return m_nFunbariHeretic;
}

int& cTaskCharPlayer::nSPCount()
{
	return m_nSPCount;
}

bool cTaskCharPlayer::IsFall(float x, float y, float z)
{
	return IsFall(Vector4(x, y, z, 0.0f));
}

void cTaskCharPlayer::ConsolValState(int state)
{
	m_nConsState = state;
}

int cTaskCharPlayer::ConsolValState()
{
	return m_nConsState;
}

int& cTaskCharPlayer::nChargeInterval()
{
	return m_nChargeInterval;
}

void cTaskCharPlayer::SetFrame(uint model, float frame)
{
	model = 0u;
	CustomKingdomUtil::Assert(pModel(model) != nullptr, "nullpo");
	if (pModel(model) != nullptr)
	{
		pModel(model)->Visible();
		pModel(model)->SetFrameAnmAll(0u, frame);
	}
}

float cTaskCharPlayer::GetFrame(uint model)
{
	model = 0u;
	if (pModel(model) != nullptr)
	{
		return pModel(model)->GetFrameAnmChr(0u);
	}
	return 0.0f;
}

void cTaskCharPlayer::Init()
{
	Constants::DefaultPlayerParam();
	::Constants::jetGeoVecUnit(vMBegin());
	InitParam(Constants::get_g_MoPlayerParam());
	OnFlag(2u);
	InitRecoord();
	m_fReverseHeigthRate = 0.0f;
	m_nPowerLevel = 0;
	m_nHoverLevel = 0;
	m_nChargeLevel = 0;
	m_nChargePowLevel = 0;
	m_nChargePowLevelOLD = 0;
	m_nChargeInterval = 0;
	m_nChargeWait = 0;
	m_nArrowTimer = 0;
	m_fReverseHeight = 0.0f;
	m_nHoverWait = 0u;
	bPlayerMoveStop() = false;
	ConsolValState(1);
	m_sPlayerPtk = new Constants::sPLAYER_CURSOR_PTK();
	m_sPlayerPtk->phPlayerArrow = nullptr;
	m_sPlayerPtk->phPlayerCursor = nullptr;
	nSPCount() = 3;
	m_nSpTimer = 0u;
	nModel() = 0;
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		SetMode(1);
		return;
	}
	OnFlag(1u);
	SetMode(7);
	m_fReverseHeight = 2.25f;
}

void cTaskCharPlayer::Term()
{
	if (m_sPlayerPtk->phPlayerArrow != nullptr)
	{
		m_sPlayerPtk->phPlayerArrow->Detach(true);
	}
	if (m_sPlayerPtk->phPlayerCursor != nullptr)
	{
		m_sPlayerPtk->phPlayerCursor->Detach(true);
	}
	if (m_pHoipoi != nullptr)
	{
		m_pHoipoi->TermOnce();
	}
	cTaskChar::Term();
}

void cTaskCharPlayer::UpdateMatrix()
{
	cKingdomModel* cKingdomModel2 = pModel();
	if (cKingdomModel2 != nullptr)
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, vMTrans());
		v[1] += 0.5f + m_fReverseHeight;
		::Constants::jetGeoVecCopy(cKingdomModel2->vTrans(), v);
		::Constants::jetGeoVecCopy(cKingdomModel2->vRotate(), vRotate());
		::Constants::jetGeoVecCopy(m_pHoipoi->vMTrans(), v);
		::Constants::jetGeoVecUnit(v);
		v = cKingdomModel2->vTrans();
		v.w = 1.0f;
		if (m_sPlayerPtk->phPlayerArrow != nullptr)
		{
			m_sPlayerPtk->phPlayerArrow->vSetTrans(v);
		}
		if (m_sPlayerPtk->phPlayerCursor != nullptr)
		{
			m_sPlayerPtk->phPlayerCursor->vSetTrans(v);
		}
	}
}

bool cTaskCharPlayer::Create(uint no)
{
	m_nPlayerNo = no;
	cKingdomModel* cKingdomModel2 = CreateModel();
	cTaskCharPlayer::sPLAYER_LIST* array[4] = {[]() { cTaskCharPlayer::sPLAYER_LIST* _o9 = new cTaskCharPlayer::sPLAYER_LIST(); _o9->Player[0] = "MDL_CHAR_PACMAN_STANDARD_1P"; _o9->Player[1] = "MDL_CHAR_PACMAN_MOBILITY_1P"; _o9->Player[2] = "MDL_CHAR_PACMAN_FORCE_1P"; _o9->Player[3] = "MDL_CHAR_PACMAN_MASTER_1P"; return _o9; }(), []() { cTaskCharPlayer::sPLAYER_LIST* _o10 = new cTaskCharPlayer::sPLAYER_LIST(); _o10->Player[0] = "MDL_CHAR_PACMAN_STANDARD_2P"; _o10->Player[1] = "MDL_CHAR_PACMAN_MOBILITY_2P"; _o10->Player[2] = "MDL_CHAR_PACMAN_FORCE_2P"; _o10->Player[3] = "MDL_CHAR_PACMAN_MASTER_2P"; return _o10; }(), []() { cTaskCharPlayer::sPLAYER_LIST* _o11 = new cTaskCharPlayer::sPLAYER_LIST(); _o11->Player[0] = "MDL_CHAR_PACMAN_STANDARD_3P"; _o11->Player[1] = "MDL_CHAR_PACMAN_MOBILITY_3P"; _o11->Player[2] = "MDL_CHAR_PACMAN_FORCE_3P"; _o11->Player[3] = "MDL_CHAR_PACMAN_MASTER_3P"; return _o11; }(), []() { cTaskCharPlayer::sPLAYER_LIST* _o12 = new cTaskCharPlayer::sPLAYER_LIST(); _o12->Player[0] = "MDL_CHAR_PACMAN_STANDARD_4P"; _o12->Player[1] = "MDL_CHAR_PACMAN_MOBILITY_4P"; _o12->Player[2] = "MDL_CHAR_PACMAN_FORCE_4P"; _o12->Player[3] = "MDL_CHAR_PACMAN_MASTER_4P"; return _o12; }()};
	if (m_nHoverLevel == 0 && m_nChargeLevel == 0)
	{
		cKingdomModel2->RegistModel(array[no]->Player[0]);
	}
	else if (m_nHoverLevel != 0 && m_nChargeLevel == 0)
	{
		cKingdomModel2->RegistModel(array[no]->Player[1]);
	}
	else if (m_nChargeLevel != 0 && m_nHoverLevel == 0)
	{
		cKingdomModel2->RegistModel(array[no]->Player[2]);
	}
	else
	{
		cKingdomModel2->RegistModel(array[no]->Player[3]);
	}
	cKingdomModel2->RegistAnm(0u, "CHA_CHAR_PACMAN_01_STANDARD");
	cKingdomModel2->SetLightSetIdx(2);
	cKingdomModel2->SetFogIdx(2);
	cKingdomModel2->IsDrawShadowDepth(true);
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		nChipColor() = (int)(10 + no);
		if (nChipColor() < 10 || nChipColor() > 13)
		{
			nChipColor() = 10;
		}
	}
	m_pHoipoi = new cTaskCharHoipoi();
	ConnectChild(m_pHoipoi);
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		m_pHoipoi->StopFrameRate();
		m_pHoipoi->InitFrame();
		if (m_sPlayerPtk->phPlayerArrow == nullptr)
		{
			m_sPlayerPtk->phPlayerArrow = Constants::SpawnEffect(34, vMTrans(), this, 0, false);
		}
		if (m_sPlayerPtk->phPlayerCursor == nullptr)
		{
			m_sPlayerPtk->phPlayerCursor = Constants::SpawnEffect((int)(42 + ID()), vMTrans(), this, 0, false);
		}
	}
	return true;
}

void cTaskCharPlayer::SetAnimation(uint AnimNo)
{
	if (m_prevAnimNo != AnimNo)
	{
		cKingdomModel* cKingdomModel2 = pModel();
		cTaskCharPlayer::sANIMATION_LIST* sANIMATION_LIST2 = AnimationName[0];
		sANIMATION_LIST2 = ((m_nHoverLevel == 0 && m_nChargeLevel == 0) ? AnimationName[0] : ((m_nHoverLevel != 0 && m_nChargeLevel == 0) ? AnimationName[2] : ((m_nChargeLevel == 0 || m_nHoverLevel != 0) ? AnimationName[3] : AnimationName[1])));
		switch (AnimNo)
		{
		case 1u:
			cKingdomModel2->RegistAnm(0u, sANIMATION_LIST2->anm[0]);
			break;
		case 9u:
			cKingdomModel2->RegistAnm(0u, sANIMATION_LIST2->anm[1]);
			break;
		case 3u:
			cKingdomModel2->RegistAnm(0u, sANIMATION_LIST2->anm[2]);
			break;
		case 10u:
			cKingdomModel2->RegistAnm(0u, sANIMATION_LIST2->anm[3]);
			break;
		case 6u:
			cKingdomModel2->RegistAnm(0u, sANIMATION_LIST2->anm[4]);
			break;
		}
		m_prevAnimNo = AnimNo;
	}
}

uint cTaskCharPlayer::WiiPadInput(const Vector4& rec, const Vector4& rec2)
{
	m_bWiiSwing = false;
	::Constants::jetGeoVecCopy(m_vInputRec[m_nRecoordPos], rec);
	::Constants::jetGeoVecCopy(m_vInputRecEx[m_nRecoordPos], rec2);
	m_nRecoordPos = (m_nRecoordPos + 1) % 6;
	m_bWiiSwing = false;
	m_bWiiSwingEx = false;
	for (uint num = 0u; num < 3; num++)
	{
		float num2 = 0.0f;
		float num3 = 0.0f;
		for (uint num4 = 0u; num4 < 6; num4++)
		{
			num2 += m_vInputRec[num4][(int)num];
		}
		for (uint num5 = 0u; num5 < 6; num5++)
		{
			num3 += m_vInputRecEx[num5][(int)num];
		}
		num2 /= 6.0f;
		num3 /= 6.0f;
		if ((double)num2 > 1.0)
		{
			m_bWiiSwing = true;
		}
		if ((double)num3 > 1.0)
		{
			m_bWiiSwingEx = true;
		}
	}
	return 1u;
}

uint cTaskCharPlayer::GetPadNo()
{
	uint num = 0u;
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		num = KingdomGlobal::MotosSys->pGetMatchingData()->Info[ID()]->nCtrl;
		CustomKingdomUtil::Assert(num < 4, "error");
	}
	else
	{
		num = 0u;
	}
	return num;
}

void cTaskCharPlayer::InitRecoord()
{
	m_nRecoordPos = 0u;
	Vector4(0.0f, -1.0f, 0.0f, 0.0f);
}

uint cTaskCharPlayer::Input()
{
	float num = 0.0f;
	float num2 = 0.0f;
	Vector4 v = Vector4::zero;
	::Constants::jetGeoVecUnit(v);
	ushort num3 = 0;
	int padNum = InGamePadManager::GetPadNum((int)GetPadNo());
	if (bTestFlag(64u))
	{
		fSpeed() = 0.0f;
		return 0u;
	}
	if (bTestFlag(512u))
	{
		return 0u;
	}
	num = (bTestFlag(384u) ? fAccelSpeedHover() : fAccelSpeed());
	Vector2 padLStickOrKey = GetPadLStickOrKey(padNum);
	float num4 = padLStickOrKey.x * 128.0f;
	float num5 = (0.0f - padLStickOrKey.y) * 128.0f;
	if (Mathf::Abs(num4) > 64.0f || Mathf::Abs(num5) > 64.0f)
	{
		num4 = num4;
		num4 = num4;
		Vector4 v1_ptr = Vector4(0.0f, 0.0f, 1.0f, 1.0f);
		v[0] = num4;
		v[1] = 0.0f;
		v[2] = num5;
		float num6 = Constants::tspVector3Length(v);
		if (CustomKingdomUtil::IF(num6))
		{
			Constants::VEC3_DIV(v, v, num6);
		}
		else
		{
			::Constants::jetGeoVecUnit(v);
		}
		if (num6 < 128.0f)
		{
			num *= num6 / 128.0f;
		}
		float num7 = Constants::Motos_P_vecGetRad(v, v1_ptr);
		vRotate()[1] = num7;
		num3 = (ushort)Constants::motosRad2Axis(num7);
		switch (num3)
		{
		case 0:
			num3 = 2;
			break;
		case 1:
			num3 = 10;
			break;
		case 2:
			num3 = 8;
			break;
		case 3:
			num3 = 9;
			break;
		case 4:
			num3 = 1;
			break;
		case 5:
			num3 = 5;
			break;
		case 6:
			num3 = 4;
			break;
		case 7:
			num3 = 6;
			break;
		}
		nModelDir() = num3;
	}
	if (!bTestFlag(2097152u))
	{
		num2 = num * (1.0f - HereticRate());
		Constants::VEC3_MUL(v, v, num2);
		Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
	}
	else
	{
		if (nPiyoTimer() != 0 && GetPadTrg(padNum, cTaskCharPlayer::EKey::eALL))
		{
			nPiyoTimer() -= 5;
		}
		num3 = 0;
	}
	AdjustMaxSpeed();
	nInputDir() = num3;
	return num3;
}

bool cTaskCharPlayer::Execute(cJetTask* pParam)
{
	if (Constants::SYSTEM_IS_PAUSE())
	{
		ChargeSoundStop();
	}
	if (bPlayerMoveStop())
	{
		return true;
	}
	cTaskChar::Execute(pParam);
	return true;
}

void cTaskCharPlayer::PresetEnv()
{
	Constants::MOTOS_NO_SOURCE("PresetEnv");
}

void cTaskCharPlayer::ChargeSoundStop()
{
	if (SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE009_CHARGE))
	{
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE009_CHARGE);
	}
	if (SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE010_CHARGE_MAX))
	{
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE010_CHARGE_MAX);
	}
	if (SoundManager::get_SE()->IsPlayinSEKind(ESEKind::MOTOS_SND_SE057_CHARGE03))
	{
		SoundManager::get_SE()->Stop(ESEKind::MOTOS_SND_SE057_CHARGE03);
	}
}

bool cTaskCharPlayer::Status()
{
	fHitPower() = Constants::get_g_PowerLevel2Power()[m_nPowerLevel];
	if (bTestFlag(12288u))
	{
		int num = 0;
		num = ((!bTestFlag(8192u)) ? m_nChargePowLevel : ChargeCnt2Level(m_nChargeCnt));
		nHereticVal() = (uint)Constants::get_g_ChargeLevel2Weight()[num];
	}
	else
	{
		nHereticVal() = nDefHereticVal();
	}
	if (!CustomKingdomUtil::IF(nHereticTimer()) && Constants::IS_MYCHAR(this))
	{
		m_nChargePowLevel = 0;
	}
	fChargePower() = Constants::get_g_ChargeLevel2Power()[m_nChargePowLevel];
	if (bTrigFlag(524288u))
	{
		ExecRebirth();
	}
	if (CustomKingdomUtil::IF(nChargeLevel()) && !CustomKingdomUtil::IF(nHoverLevel()) && nModel() != 2)
	{
		nModel() = 2;
	}
	if (!CustomKingdomUtil::IF(nChargeLevel()) && CustomKingdomUtil::IF(nHoverLevel()) && nModel() != 1)
	{
		nModel() = 1;
	}
	if (CustomKingdomUtil::IF(nChargeLevel()) && CustomKingdomUtil::IF(nHoverLevel()) && nModel() != 3)
	{
		nModel() = 3;
	}
	if (bTestFlag(384u) && !bTestFlagOLD(384u) && (!bTestFlag(262144u) || !bTestFlagOLD(16384u)))
	{
		Constants::SpawnEffect(16, vMTrans());
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE017_JUMP);
	}
	if (bTestFlag(256u) && !bTestFlag(524288u) && m_nHoverLevel > 1)
	{
		if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
		{
			int num2 = (int)((long)Constants::get_g_HoverLevel2Time()[m_nHoverLevel] - (long)nHoverTimer());
			if ((!CustomKingdomUtil::IF(num2) || !CustomKingdomUtil::IF(num2 % 20)) && !KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsFall(vMTrans()))
			{
				Constants::SpawnEffect(31, vMTrans());
			}
		}
		else
		{
			CustomKingdomUtil::IF(Constants::get_GAME_COUNTER() % 20);
		}
	}
	if (bTrigFlag(524288u))
	{
		Vector4 v;
		::Constants::jetGeoVecCopy(v, vMTrans());
		v[1] += m_fReverseHeight + 0.5f;
		Constants::SpawnEffect(23, v);
	}
	if (!IsFall() && !bTestFlag(384u) && bTestFlagOLD(384u))
	{
		Constants::SpawnEffect(17, vMTrans());
		SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE019_TOUCHDOWN);
		Vector4 v2 = Vector4::zero;
		::Constants::jetGeoVecUnit(v2);
	}
	if (bTestFlag(8192u))
	{
		if (!bTestFlagOLD(8192u))
		{
			nEffectTimer() = 0;
			nChargeInterval() = (int)(28.0f + 0.2f * (float)Constants::get_g_ChargeLevel2Time()[m_nChargeLevel]);
			if (nChargeInterval() <= 0)
			{
				nChargeInterval() = 8;
			}
		}
		if (nEffectTimer()++ % nChargeInterval() == 0)
		{
			int num3 = Constants::get_g_ChargeLevel2Time()[m_nChargeLevel] - m_nChargeCnt;
			if (num3 < 0)
			{
				num3 = 0;
			}
			nChargeInterval() = (int)(28.0f + 0.2f * (float)num3);
			if (nChargeInterval() <= 0)
			{
				nChargeInterval() = 8;
			}
			if (m_nChargeCnt >= Constants::get_g_ChargeLevel2Time()[m_nChargeLevel])
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE010_CHARGE_MAX, 0.0f, 1.0f, SoundSe::EMixerType::eNORMAL, SoundSeObject::ESeType::eNORMAL, 1.0f, SoundSe::ESameKindCheck::eEARLY);
			}
			SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE009_CHARGE);
			Constants::SpawnEffect(14, vMTrans(), this);
			nEffectTimer() = 1;
		}
	}
	if (bTestFlag(4096u))
	{
		if (!bTestFlagOLD(4096u))
		{
			ChargeSoundStop();
			if (bTestFlag(1048576u))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE008_DASH);
			}
			else
			{
				ChargeSoundStop();
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE008_DASH);
			}
		}
		if (!bTestFlag(131136u))
		{
			CustomKingdomUtil::IF(nEffectTimer()++ % 4);
		}
	}
	if (Constants::IS_MYCHAR(this))
	{
		ConsolValState(1);
		if (bTestFlag(8192u))
		{
			ConsolValState(2);
		}
		if (bTestFlag(384u))
		{
			ConsolValState(3);
		}
		if (!bTestFlag(384u) && bTestFlagOLD(384u))
		{
			ConsolValState(4);
		}
		if (bRefFlag())
		{
			ConsolValState(5);
		}
		if (bRefFlag() && bTestFlag(4096u))
		{
			ConsolValState(6);
		}
		if (bTestFlag(64u) && !bTestFlag(16384u))
		{
			ConsolValState(7);
		}
		if (bTestFlag(16384u))
		{
			ConsolValState(8);
		}
		if (bTrigFlag(524288u))
		{
			ConsolValState(9);
		}
	}
	return cTaskChar::Status();
}

bool cTaskCharPlayer::ActionBoth()
{
	return true;
}

bool cTaskCharPlayer::ActionLocal()
{
	Input();
	return ExecModeFunc() != 0;
}

bool cTaskCharPlayer::ActionOther()
{
	return true;
}

bool cTaskCharPlayer::TaikiAction()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		SetMode(7);
		vMTrans()[2] = 49.0f;
	}
	return true;
}

void cTaskCharPlayer::ExecGivePoints(int id, int add_point, int sub_point)
{
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return;
	}
	cTaskChar* cTaskChar2 = KingdomGlobal::g_pMotosGame->pChars((uint)id);
	if (cTaskChar2 != nullptr && cTaskChar2->bTestFlag(2u))
	{
		pGivePoints()->m_nTargetID = (short)cTaskChar2->ID();
		if (add_point < 0)
		{
			pGivePoints()->m_nAddPoints = (ushort)((float)KingdomGlobal::g_pMotosGame->nPoints((int)ID()) * Constants::get_MO_VS_ROB_POINT_RATE());
		}
		else
		{
			pGivePoints()->m_nAddPoints = (ushort)add_point;
		}
		if (sub_point < 0)
		{
			pGivePoints()->m_nSubPoints = (ushort)((float)KingdomGlobal::g_pMotosGame->nPoints((int)ID()) * Constants::get_MO_VS_FALL_POINT_RATE());
		}
		else
		{
			pGivePoints()->m_nSubPoints = (ushort)sub_point;
		}
		pGivePoints()->m_nGameCounter = Constants::get_GAME_COUNTER();
	}
	else
	{
		pGivePoints()->m_nTargetID = 128;
		pGivePoints()->m_nAddPoints = 0;
		if (sub_point < 0)
		{
			pGivePoints()->m_nSubPoints = (ushort)((float)KingdomGlobal::g_pMotosGame->nPoints((int)ID()) * Constants::get_MO_VS_FALL_POINT_RATE());
		}
		else
		{
			pGivePoints()->m_nSubPoints = (ushort)sub_point;
		}
		pGivePoints()->m_nGameCounter = Constants::get_GAME_COUNTER();
	}
}

int cTaskCharPlayer::ChargeCnt2Level(int n)
{
	int num = 0;
	for (num = 0; n > Constants::get_g_ChargeLevel2Time()[num] && num < 10; num++)
	{
	}
	return num;
}

void cTaskCharPlayer::InitParam(MO_CHAR_PARAMS* pParam)
{
	cTaskChar::InitParam(pParam);
	m_PLParam = new Constants::MO_PLAYER_PARAMS();
	m_PLParam->m_fMaxSpeedHover = pParam->m_fExParam[0];
	m_PLParam->m_fAccelSpeedHover = pParam->m_fExParam[1];
	m_PLParam->m_fDecelSpeedHover = pParam->m_fExParam[2];
}

void cTaskCharPlayer::AdjustMaxSpeed()
{
	float num = Constants::tspVector3Length(vSpeed());
	if (!bTestFlag(384u))
	{
		float num2 = fMaxSpeed();
		if (Constants::IS_LANPLAY())
		{
			int num3 = KingdomGlobal::g_pMotosGame->nPoints((int)ID());
			num3 /= Constants::get_MO_VS_POINT_DECEL_POINT();
			float num4 = 1.0f - Constants::get_MO_VS_POINT_DECEL_DECEL() * (float)num3;
			if (num4 < 0.7f)
			{
				num4 = 0.7f;
			}
			num2 *= num4;
		}
		if (num > num2)
		{
			num = num2 / num;
			Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
		}
	}
	else
	{
		float num2 = fMaxSpeedHover();
		if (num > num2)
		{
			num = num2 / num;
			Constants::VEC3_MUL(vSpeed(), vSpeed(), num);
		}
	}
	if (!bTestFlag(64u))
	{
		fSpeed() = num;
	}
	else
	{
		fSpeed() = 0.0f;
	}
}

void cTaskCharPlayer::DecelSpeed(int input_dir)
{
	if (!bTestFlag(384u))
	{
		if (!CustomKingdomUtil::IF(input_dir & 0xC))
		{
			vSpeed()[0] *= fDecelSpeed();
		}
		if (!CustomKingdomUtil::IF(input_dir & 3))
		{
			vSpeed()[2] *= fDecelSpeed();
		}
	}
	else
	{
		vSpeed()[0] *= fDecelSpeedHover();
		vSpeed()[2] *= fDecelSpeedHover();
	}
}

void cTaskCharPlayer::RegistPacket()
{
}

void cTaskCharPlayer::SyncPacket()
{
}

float cTaskCharPlayer::ConsolValPower()
{
	return nPowerLevel();
}

float cTaskCharPlayer::ConsolValHover()
{
	if (GetMode() == 3)
	{
		float num = (float)nHoverTimer() / (float)Constants::get_g_HoverLevel2Time()[nHoverLevel()];
		return (float)nHoverLevel() * num;
	}
	return nHoverLevel();
}

float cTaskCharPlayer::ConsolValCharge()
{
	if (GetMode() == 4)
	{
		float num = (float)m_nChargeCnt / (float)Constants::get_g_ChargeLevel2Time()[m_nChargeLevel];
		return (float)nChargeLevel() * (1.0f - num);
	}
	return nChargeLevel();
}

void cTaskCharPlayer::UseItem()
{
	CustomKingdomUtil::Assert(ID() < 4, "error");
	nPowerLevel() = KingdomGlobal::pMotosGameSys->KeepData()[ID()]->m_nUseItem[1];
	nHoverLevel() = KingdomGlobal::pMotosGameSys->KeepData()[ID()]->m_nUseItem[2];
	nChargeLevel() = KingdomGlobal::pMotosGameSys->KeepData()[ID()]->m_nUseItem[3];
}

bool cTaskCharPlayer::ExecRebirth()
{
	Step() = 0;
	Constants::VEC3_SET(vSpeed(), 0.0f, 0.0f, 0.0f);
	return true;
}

bool cTaskCharPlayer::ChangeFunbari()
{
	int padNum = InGamePadManager::GetPadNum((int)GetPadNo());
	int padDir = GetPadDir(padNum);
	nFunbariHeretic() = (int)nHereticTimer();
	if (HereticRate() == 0.0f && !CustomKingdomUtil::IF(padDir & nFallDir()) && GetMode() != 3 && GetMode() != 7)
	{
		SetMode(5);
		::Constants::jetGeoVecUnit(vSpeed());
		nHereticTimer() = 0u;
		return true;
	}
	if (bTestFlag(1024u))
	{
		ExecGivePoints(nLastRefID());
	}
	else
	{
		ExecGivePoints(128);
	}
	nLastRefID() = -1;
	ChangeFallMove();
	SetMode(6);
	ChargeSoundStop();
	::Constants::jetGeoVecUnit(vSpeed());
	nHereticTimer() = 0u;
	return false;
}

bool cTaskCharPlayer::ModeFunc(int nMes, cJetTask* pListC, cJetTask* pScene)
{
	Constants::MOTOS_CAUTION("ModeFunc");
	int padNum = InGamePadManager::GetPadNum((int)GetPadNo());
	if (m_nSpTimer != 0)
	{
		m_nSpTimer--;
		if (m_nSpTimer > 15)
		{
			Constants::SpawnEffect(15, vMTrans());
		}
	}
	if (GetMode() != 0)
	{
		if (1 == GetMode())
		{
			if (2 == nMes)
			{
				ChargeSoundStop();
				Rotate2Dir(vRotate()[1]);
				m_nArrowTimer = 0;
				return true;
			}
			if (3 == nMes)
			{
				return true;
			}
			if (1 == nMes)
			{
				if (IsFall())
				{
					ChangeFunbari();
				}
				else if (!bTestFlag(2097152u))
				{
					if (AnimateModel(0u))
					{
						SetAnimation(1u);
					}
					if (CustomKingdomUtil::IF(nHoverLevel()) && InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_CIRCLE, false) && !bTestFlag(128u))
					{
						SetMode(3);
						OnFlag(128u);
						vSpeed()[0] /= 4.0f;
						vSpeed()[2] /= 4.0f;
					}
					else if (KingdomGlobal::g_pMotosGame->bMultiPlay() && CustomKingdomUtil::IF(nSPCount()) && m_nSpTimer == 0 && InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_TRIANGLE, false))
					{
						m_nSpTimer = 30u;
						nSPCount()--;
						SetMode(8);
						Constants::SpawnEffect(16, vMTrans());
						Constants::SpawnEffect(2, vMTrans());
						Constants::SpawnEffect(2, vMTrans());
					}
					else if (CustomKingdomUtil::IF(nChargeLevel()) && InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_CROSS, false))
					{
						SetMode(4);
					}
					if (Constants::IS_LANPLAY())
					{
						if (!CustomKingdomUtil::IF(GetPadDir(padNum)))
						{
							if (++m_nArrowTimer == Constants::FPS_TIME(2.0f))
							{
								Constants::SpawnEffect(34, vMTrans());
							}
						}
						else
						{
							m_nArrowTimer = 0;
						}
					}
				}
				return true;
			}
		}
		else if (2 == GetMode())
		{
			if (2 == nMes)
			{
				return true;
			}
			if (3 == nMes)
			{
				KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->StrikePanel((int)ID(), 1, fHitSize(), vMTrans());
				vSpeed()[0] = 0.0f;
				vSpeed()[2] = 0.0f;
				OffFlag(128u);
				return true;
			}
			if (1 == nMes)
			{
				if (!AnimateModel(0u))
				{
					SetMode(1);
				}
				return true;
			}
		}
		else if (3 == GetMode())
		{
			if (2 == nMes)
			{
				m_bHoiPoiRelease = false;
				nHoverTimer() = (uint)Constants::get_g_HoverLevel2Time()[m_nHoverLevel];
				Step() = 0;
				SetAnimation(9u);
				m_nHoverWait = 0u;
				return true;
			}
			if (3 == nMes)
			{
				nHoverTimer() = 0u;
				KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->StrikePanel((int)ID(), 1, fHitSize() * 0.7f, vMTrans());
				vSpeed()[0] = 0.0f;
				vSpeed()[2] = 0.0f;
				return true;
			}
			if (1 == nMes)
			{
				int num = 0;
				Vector4 pOut = Vector4();
				cTaskCharBoss* cTaskCharBoss2 = nullptr;
				nPiyoTimer() = 0;
				if (Constants::IS_BOSS_STAGE())
				{
					cTaskCharBoss2 = static_cast<cTaskCharBoss*>(KingdomGlobal::g_pMotosGame->pBoss());
				}
				Vector4 _out;
				switch (Step())
				{
				case 0:
					if (AnimateModel(0u) && m_nHoverWait++ > 10)
					{
						m_nHoverWait = 0u;
						Step()++;
						OffFlag(128u);
						OnFlag(256u);
						SetAnimation(3u);
					}
					break;
				case 1:
					if (CustomKingdomUtil::IF(nHoverTimer()))
					{
						nHoverTimer()--;
					}
					if (!m_bHoiPoiRelease && InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_CIRCLE, false) && CustomKingdomUtil::IF(nHoverTimer()))
					{
						break;
					}
					m_bHoiPoiRelease = true;
					if (m_pHoipoi->pModel()->IsVisible())
					{
						m_pHoipoi->StopFrameRate();
					}
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						int num3 = 0;
						if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eUP))
						{
							num3 |= 1;
						}
						else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eDOWN))
						{
							num3 |= 2;
						}
						if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eLEFT))
						{
							num3 |= 4;
						}
						else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eRIGHT))
						{
							num3 |= 8;
						}
						if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->ForceAdjustPanel(pOut, vMTrans(), 0.01f, num3) != 0)
						{
							Constants::VEC3_SUB(_out, pOut, vMTrans());
							_out[1] = 0.0f;
							float num2 = Constants::tspVector3Length(_out);
							if ((double)num2 > 0.1)
							{
								Constants::VEC3_DIV(_out, _out, num2);
								Constants::VEC3_MUL(_out, _out, 0.1f);
							}
							vSpeed()[0] = _out[0];
							vSpeed()[2] = _out[2];
							nHereticTimer() = 0u;
							OnFlag(512u);
						}
					}
					else if (cTaskCharBoss2 != nullptr && cTaskCharBoss2->nHp() > 0 && KingdomGlobal::g_pMotosGame->pHitMan()->HitCheckSimple(this, cTaskCharBoss2, {}, {}))
					{
						if (RefChar() != (int)cTaskCharBoss2->GrobalID())
						{
							ExecReflectSub(Constants::motosDir2Vector(nModelDir()), 0.5f, 0.2f);
							OnFlag(512u);
						}
					}
					else
					{
						nHereticTimer() = 0u;
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
						Step()++;
						OnFlag(128u);
						OffFlag(256u);
						SetAnimation(10u);
					}
					break;
				case 2:
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						OffFlag(512u);
						Step()--;
						break;
					}
					if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut, vMTrans(), fWHitSize() + 0.05f, 1) != 0)
					{
						Constants::VEC3_SUB(_out, pOut, vMTrans());
						_out[1] = 0.0f;
						float num2 = Constants::tspVector3Length(_out);
						if ((double)num2 > 0.1)
						{
							Constants::VEC3_DIV(_out, _out, num2);
							Constants::VEC3_MUL(_out, _out, 0.1f);
						}
						vSpeed()[0] = _out[0];
						vSpeed()[2] = _out[2];
						nHereticTimer() = 0u;
						OnFlag(512u);
					}
					else if (bTestFlag(512u))
					{
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
						nHereticTimer() = 0u;
					}
					if (AnimateModel(0u))
					{
						if (IsFall())
						{
							ChangeFunbari();
						}
						else
						{
							SetMode(1);
						}
						OffFlag(128u);
						OffFlag(512u);
					}
					break;
				}
				return true;
			}
		}
		else if (6 == GetMode())
		{
			if (2 == nMes)
			{
				SetAnimation(1u);
				return true;
			}
			if (3 == nMes)
			{
				OffFlag(64u);
				::Constants::jetGeoVecUnit(vSpeed());
				ChargeSoundStop();
				return true;
			}
			if (1 == nMes)
			{
				ChargeSoundStop();
				return true;
			}
		}
		else if (4 == GetMode())
		{
			if (2 == nMes)
			{
				OnFlag(8192u);
				OffFlag(4096u);
				Step() = 0;
				m_nChargeCnt = 0;
				m_nChargeWait = 0;
				::Constants::jetGeoVecUnit(vSpeed());
				return true;
			}
			if (3 == nMes)
			{
				m_nChargeCnt = 0;
				OffFlag(512u);
				fMaxSpeed() = fDefMaxSpeed();
				OffFlag(8192u);
				return true;
			}
			if (1 == nMes)
			{
				if (IsFall())
				{
					ChangeFunbari();
				}
				else
				{
					switch (Step())
					{
					case 0:
						if (bTestFlag(2097152u))
						{
							ChargeSoundStop();
							SetMode(1);
						}
						if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_CROSS, false))
						{
							vMTrans()[1] = 0.0f;
							Step()++;
							break;
						}
						if (++m_nChargeWait > 15 && m_nChargeCnt < Constants::get_g_ChargeLevel2Time()[m_nChargeLevel])
						{
							m_nChargeCnt++;
						}
						vMTrans()[1] = Constants::MOTOS_RANDRANGEF(0.0f, 0.2f);
						break;
					case 1:
						if (m_nChargeCnt != 0)
						{
							ChargeSoundStop();
							Dir2Rotate(nModelDir());
							m_nChargePowLevelOLD = (nChargePowLevel() = ChargeCnt2Level(m_nChargeCnt));
							ExecReflectSub(Constants::motosDir2Vector(nModelDir()), Constants::get_g_ChargeLevel2Heretic()[nChargePowLevel()], Constants::get_g_ChargeLevel2MyRush()[m_nChargePowLevel]);
							OnFlag(4096u);
							OnFlag(1048576u);
						}
						SetMode(1);
						break;
					}
				}
				return true;
			}
		}
		else if (5 == GetMode())
		{
			if (2 == nMes)
			{
				nTimer() = 15;
				::Constants::jetGeoVecUnit(vSpeed());
				OnFlag(512u);
				nFunbariMeter() = 5;
				return true;
			}
			if (3 == nMes)
			{
				vMTrans()[1] = 0.0f;
				nTimer() = 0;
				OffFlag(512u);
				return true;
			}
			if (1 == nMes)
			{
				if (CustomKingdomUtil::IF(nHoverLevel()) && InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_CIRCLE, false))
				{
					SetMode(3);
					OnFlag(128u);
					vSpeed()[0] /= 4.0f;
					vSpeed()[2] /= 4.0f;
				}
				else
				{
					if (!CustomKingdomUtil::IF(nTimer()--))
					{
						if (IsFall())
						{
							if (nFunbariHeretic() != 0)
							{
								ExecGivePoints(nLastRefID());
							}
							else
							{
								ExecGivePoints(128);
							}
							nLastRefID() = -1;
							ChangeFallMove();
							SetMode(6);
							::Constants::jetGeoVecUnit(vSpeed());
						}
						else
						{
							SetMode(1);
						}
					}
					else
					{
						ushort num4 = nFallDir();
						int num5 = Constants::motosReverseDir(num4);
						Vector2 padLStickOrKey = GetPadLStickOrKey(padNum);
						float num6 = padLStickOrKey.x * 128.0f;
						float num7 = (0.0f - padLStickOrKey.y) * 128.0f;
						uint num8 = 0u;
						if (num6 < -64.0f)
						{
							num8 |= 4;
						}
						else if (num6 > 64.0f)
						{
							num8 |= 8;
						}
						if (num7 < -64.0f)
						{
							num8 |= 1;
						}
						else if (num7 > 64.0f)
						{
							num8 |= 2;
						}
						Vector4 v;
						if (!IsFall())
						{
							SetMode(1);
						}
						else if (CustomKingdomUtil::IF(num8 & num4))
						{
							nTimer() = 0;
						}
						else if (CustomKingdomUtil::IF((long)num8 & (long)num5))
						{
							if (++nFunbariMeter() > 10)
							{
								num4 = 0;
								if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eUP))
								{
									num4 |= 1;
								}
								else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eDOWN))
								{
									num4 |= 2;
								}
								if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eLEFT))
								{
									num4 |= 4;
								}
								else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eRIGHT))
								{
									num4 |= 8;
								}
								::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(num4));
								Constants::VEC3_MUL(v, v, 0.1f);
								::Constants::jetGeoVecCopy(vSpeed(), v);
							}
							nInputDir() = num4;
						}
						else
						{
							nFunbariMeter()--;
							::Constants::jetGeoVecCopy(v, Constants::motosDir2Vector(num5));
							Constants::VEC3_MUL(v, v, 0.035f);
							Constants::VEC3_ADD(vSpeed(), vSpeed(), v);
							nInputDir() = num4;
						}
					}
					vMTrans()[1] = Constants::MOTOS_RANDRANGEF(0.0f, 0.1f);
				}
				return true;
			}
		}
		else if (7 == GetMode())
		{
			if (2 == nMes)
			{
				vRotate()[1] = 0.0f;
				Rotate2Dir(0.0f);
				OffFlag(1u);
				Step() = 0;
				m_fReverseHeight = 2.25f;
				if (OldMode() != 0)
				{
					::Constants::jetGeoVecCopy(vMTrans(), vMBeginTrans());
				}
				OnFlag(256u);
				OnFlag(786432u);
				OffFlag(128u);
				(void)(Constants::get_MO_VS_PLAYER_MUTEKI());
				nMutekiTimer() = Constants::get_MO_VS_PLAYER_MUTEKI();
				nTimer() = Constants::MO_VS_REBIRTH_LIMIT();
				m_bHoiPoiRelease = false;
				m_fReverseHeigthRate = 0.0f;
				m_pHoipoi->StopFrameRate();
				m_pHoipoi->InitFrame();
				return true;
			}
			if (3 == nMes)
			{
				vSpeed()[0] = 0.0f;
				vSpeed()[2] = 0.0f;
				OffFlag(524288u);
				m_bHoiPoiRelease = false;
				return true;
			}
			if (1 == nMes)
			{
				int num9 = 0;
				Vector4 pOut2 = Vector4();
				m_fReverseHeigthRate += 0.05f;
				m_pHoipoi->pModel()->vScale()[0] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 20.0f;
				m_pHoipoi->pModel()->vScale()[1] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 20.0f;
				m_pHoipoi->pModel()->vScale()[2] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 20.0f;
				pModel()->vScale()[0] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 10.0f;
				pModel()->vScale()[1] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 10.0f;
				pModel()->vScale()[2] = 1.0f + Mathf::Sin(m_fReverseHeigthRate) / 10.0f;
				if (Step() < 4)
				{
					m_fReverseHeight = 2.25f + Mathf::Sin(m_fReverseHeigthRate) / 3.0f;
				}
				Vector4 _out2;
				switch (Step())
				{
				case 0:
					if (CustomKingdomUtil::IF(nTimer()))
					{
						nTimer()--;
					}
					if (GetPadVSRebirth(padNum))
					{
						m_bHoiPoiRelease = true;
					}
					if (CustomKingdomUtil::IF(nTimer()) && !m_bHoiPoiRelease)
					{
						break;
					}
					if (m_pHoipoi->pModel()->IsVisible())
					{
						m_pHoipoi->StopFrameRate();
					}
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						int num11 = 0;
						if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eUP))
						{
							num11 |= 1;
						}
						else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eDOWN))
						{
							num11 |= 2;
						}
						if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eLEFT))
						{
							num11 |= 4;
						}
						else if (GetPadCntOrTrg(padNum, cTaskCharPlayer::EKey::eRIGHT))
						{
							num11 |= 8;
						}
						if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->ForceAdjustPanel(pOut2, vMTrans(), 0.01f, num11) != 0)
						{
							Constants::VEC3_SUB(_out2, pOut2, vMTrans());
							_out2[1] = 0.0f;
							float num10 = Constants::tspVector3Length(_out2);
							if ((double)num10 > 0.1)
							{
								Constants::VEC3_DIV(_out2, _out2, num10);
								Constants::VEC3_MUL(_out2, _out2, 0.1f);
							}
							vSpeed()[0] = _out2[0];
							vSpeed()[2] = _out2[2];
							nHereticTimer() = 0u;
							OnFlag(512u);
						}
					}
					else
					{
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
						Step()++;
						OnFlag(128u);
						OffFlag(256u);
					}
					break;
				case 1:
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						OffFlag(128u);
						OffFlag(512u);
						Step()--;
						break;
					}
					if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut2, vMTrans(), fWHitSize() + 0.05f, 1) != 0)
					{
						Constants::VEC3_SUB(_out2, pOut2, vMTrans());
						_out2[1] = 0.0f;
						float num10 = Constants::tspVector3Length(_out2);
						if ((double)num10 > 0.1)
						{
							Constants::VEC3_DIV(_out2, _out2, num10);
							Constants::VEC3_MUL(_out2, _out2, 0.1f);
						}
						vSpeed()[0] = _out2[0];
						vSpeed()[2] = _out2[2];
						nHereticTimer() = 0u;
						OnFlag(512u);
					}
					else if (bTestFlag(512u))
					{
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
					}
					nHereticTimer() = 0u;
					Step()++;
					break;
				case 2:
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						OffFlag(128u);
						OffFlag(512u);
						Step() -= 2;
					}
					else if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut2, vMTrans(), fWHitSize() + 0.05f, 1) != 0)
					{
						Constants::VEC3_SUB(_out2, pOut2, vMTrans());
						_out2[1] = 0.0f;
						float num10 = Constants::tspVector3Length(_out2);
						if ((double)num10 > 0.1)
						{
							Constants::VEC3_DIV(_out2, _out2, num10);
							Constants::VEC3_MUL(_out2, _out2, 0.1f);
						}
						vSpeed()[0] = _out2[0];
						vSpeed()[2] = _out2[2];
						nHereticTimer() = 0u;
						OnFlag(512u);
					}
					else
					{
						if (bTestFlag(512u))
						{
							vSpeed()[0] = 0.0f;
							vSpeed()[2] = 0.0f;
							m_pHoipoi->LoadFrameRate();
						}
						nHereticTimer() = 0u;
						Step()++;
						m_pHoipoi->LoadFrameRate();
					}
					break;
				case 3:
					pModel()->vScale()[0] += (1.0f - pModel()->vScale()[0]) / 10.0f;
					pModel()->vScale()[1] += (1.0f - pModel()->vScale()[1]) / 10.0f;
					pModel()->vScale()[2] += (1.0f - pModel()->vScale()[2]) / 10.0f;
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						OffFlag(128u);
						OffFlag(512u);
						break;
					}
					if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut2, vMTrans(), fWHitSize() + 0.05f, 1) != 0)
					{
						Constants::VEC3_SUB(_out2, pOut2, vMTrans());
						_out2[1] = 0.0f;
						float num10 = Constants::tspVector3Length(_out2);
						if ((double)num10 > 0.1)
						{
							Constants::VEC3_DIV(_out2, _out2, num10);
							Constants::VEC3_MUL(_out2, _out2, 0.1f);
						}
						vSpeed()[0] = _out2[0];
						vSpeed()[2] = _out2[2];
						nHereticTimer() = 0u;
						OnFlag(512u);
						break;
					}
					if (bTestFlag(512u))
					{
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
					}
					nHereticTimer() = 0u;
					if (m_pHoipoi->AnimateModel(0u))
					{
						if (IsFall())
						{
							ChangeFunbari();
						}
						else
						{
							Step()++;
						}
						OffFlag(512u);
					}
					break;
				case 4:
					pModel()->vScale()[0] += (1.0f - pModel()->vScale()[0]) / 10.0f;
					pModel()->vScale()[1] += (1.0f - pModel()->vScale()[1]) / 10.0f;
					pModel()->vScale()[2] += (1.0f - pModel()->vScale()[2]) / 10.0f;
					if (!KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->IsMove(vMTrans()))
					{
						OffFlag(128u);
						OffFlag(512u);
						break;
					}
					if (KingdomGlobal::g_pMotosGame->pMap()->pTaskPanelMan()->AdjustPanel(pOut2, vMTrans(), fWHitSize() + 0.05f, 1) != 0)
					{
						Constants::VEC3_SUB(_out2, pOut2, vMTrans());
						_out2[1] = 0.0f;
						float num10 = Constants::tspVector3Length(_out2);
						if ((double)num10 > 0.1)
						{
							Constants::VEC3_DIV(_out2, _out2, num10);
							Constants::VEC3_MUL(_out2, _out2, 0.1f);
						}
						vSpeed()[0] = _out2[0];
						vSpeed()[2] = _out2[2];
						nHereticTimer() = 0u;
						OnFlag(512u);
						break;
					}
					if (bTestFlag(512u))
					{
						vSpeed()[0] = 0.0f;
						vSpeed()[2] = 0.0f;
					}
					nHereticTimer() = 0u;
					m_fReverseHeight -= 0.15f;
					if (m_fReverseHeight <= 0.0f)
					{
						OffFlag(512u);
						pModel()->vScale()[0] = 1.0f;
						pModel()->vScale()[1] = 1.0f;
						pModel()->vScale()[2] = 1.0f;
						m_fReverseHeight = 0.0f;
						SetMode(1);
						OffFlag(128u);
					}
					break;
				}
				return true;
			}
		}
		else if (8 == GetMode())
		{
			if (2 == nMes)
			{
				return true;
			}
			if (3 == nMes)
			{
				OnFlag(4096u);
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE029_SPECIAL);
				OnFlag(1048576u);
				return true;
			}
			if (1 == nMes)
			{
				Dir2Rotate(nModelDir());
				m_nChargePowLevelOLD = (nChargePowLevel() = 10);
				fChargePower() = Constants::get_g_ChargeLevel2Power()[m_nChargePowLevel];
				ExecReflectSub(Constants::motosDir2Vector(nModelDir()), Constants::get_g_ChargeLevel2Heretic()[nChargePowLevel()], Constants::get_g_ChargeLevel2MyRush()[m_nChargePowLevel]);
				SetMode(1);
				return true;
			}
		}
	}
	return true;
}

bool cTaskCharPlayer::IsFall(Nullable<Vector4> pVec)
{
	if (KingdomGlobal::MotosSys->IsDebugMuteki())
	{
		return false;
	}
	return cTaskChar::IsFall(pVec);
}

bool cTaskCharPlayer::RegistPreset()
{
	if (CustomKingdomUtil::IF(nChargeLevel()))
	{
		CustomKingdomUtil::IF(nHoverLevel());
	}
	if (!CustomKingdomUtil::IF(nChargeLevel()))
	{
		CustomKingdomUtil::IF(nHoverLevel());
	}
	if (CustomKingdomUtil::IF(nChargeLevel()))
	{
		CustomKingdomUtil::IF(nHoverLevel());
	}
	return true;
}

void cTaskCharPlayer::CreateDashEffect()
{
	Constants::MOTOS_NO_SOURCE("CreateDashEffect");
}

void cTaskCharPlayer::RefAttacked(int nParts, int nGrobalID)
{
	int mode = GetMode();
	if (mode == 7)
	{
		nTimer() = 0;
	}
}

float cTaskCharPlayer::GetHitPower(int nPart, int nOtherID)
{
	if (!bTestFlagOLD(4096u))
	{
		return fHitPower();
	}
	return Constants::get_g_PowerLevel2Power()[0];
}

bool cTaskCharPlayer::GetPadCnt(int padNum, cTaskCharPlayer::EKey key)
{
	switch (key)
	{
	case cTaskCharPlayer::EKey::eUP:
		if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_UP, false))
		{
			return InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP, false);
		}
		return true;
	case cTaskCharPlayer::EKey::eDOWN:
		if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_DOWN, false))
		{
			return InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN, false);
		}
		return true;
	case cTaskCharPlayer::EKey::eLEFT:
		if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LEFT, false))
		{
			return InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT, false);
		}
		return true;
	case cTaskCharPlayer::EKey::eRIGHT:
		if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_RIGHT, false))
		{
			return InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT, false);
		}
		return true;
	default:
		return false;
	}
}

bool cTaskCharPlayer::GetPadTrg(int padNum, cTaskCharPlayer::EKey key)
{
	switch (key)
	{
	case cTaskCharPlayer::EKey::eUP:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_UP))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_UP);
		}
		return true;
	case cTaskCharPlayer::EKey::eDOWN:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_DOWN))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_DOWN);
		}
		return true;
	case cTaskCharPlayer::EKey::eLEFT:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LEFT))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT);
		}
		return true;
	case cTaskCharPlayer::EKey::eRIGHT:
		if (!InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_RIGHT))
		{
			return InGamePadManager::GetKeyDown(padNum, PadDefine::EPadKeycode::eKEY_LSTICK_RIGHT);
		}
		return true;
	case cTaskCharPlayer::EKey::eALL:
		if (!GetPadTrg(padNum, cTaskCharPlayer::EKey::eUP) && !GetPadTrg(padNum, cTaskCharPlayer::EKey::eDOWN) && !GetPadTrg(padNum, cTaskCharPlayer::EKey::eLEFT))
		{
			return GetPadTrg(padNum, cTaskCharPlayer::EKey::eRIGHT);
		}
		return true;
	default:
		return false;
	}
}

bool cTaskCharPlayer::GetPadCntOrTrg(int padNum, cTaskCharPlayer::EKey key)
{
	if (!GetPadCnt(padNum, key))
	{
		return GetPadTrg(padNum, key);
	}
	return true;
}

bool cTaskCharPlayer::GetPadVSRebirth(int padNum)
{
	if (!InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_CIRCLE) && !InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_CROSS) && !InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_SQUARE))
	{
		return InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_TRIANGLE);
	}
	return true;
}

int cTaskCharPlayer::GetPadDir(int padNum)
{
	int num = 0;
	if (GetPadCnt(padNum, cTaskCharPlayer::EKey::eUP))
	{
		num |= 1;
	}
	if (GetPadCnt(padNum, cTaskCharPlayer::EKey::eDOWN))
	{
		num |= 2;
	}
	if (GetPadCnt(padNum, cTaskCharPlayer::EKey::eLEFT))
	{
		num |= 4;
	}
	if (GetPadCnt(padNum, cTaskCharPlayer::EKey::eRIGHT))
	{
		num |= 8;
	}
	return num;
}

Vector2 cTaskCharPlayer::GetPadLStickOrKey(int padNum)
{
	Vector2 zero = Vector2::zero;
	if (InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_UP, false))
	{
		zero.y = 1.0f;
	}
	else if (InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_DOWN, false))
	{
		zero.y = -1.0f;
	}
	if (InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_LEFT, false))
	{
		zero.x = -1.0f;
	}
	else if (InGamePadManager::GetKey(padNum, PadDefine::EPadKeycode::eKEY_RIGHT, false))
	{
		zero.x = 1.0f;
	}
	Vector2 stick = InGamePadManager::GetStick(padNum, PadDefine::scLSTICK, false);
	if (Mathf::Approximately(zero.x, 0.0f))
	{
		zero.x = stick.x;
	}
	if (Mathf::Approximately(zero.y, 0.0f))
	{
		zero.y = stick.y;
	}
	return zero;
}

}
