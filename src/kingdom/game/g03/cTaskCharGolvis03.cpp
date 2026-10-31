#include "kingdom/game/g03/cPacnRollLoopSndMan.hpp"
#include "kingdom/game/g03/cTaskCharGolvis03.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <SoundManager.hpp>

namespace kingdom::game::g03
{

const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind00[5] = {[]() { Constants::LAND_ATTACK_INFO* _o1 = new Constants::LAND_ATTACK_INFO(); _o1->m_nFrame = 0u; _o1->m_nKind = 1u; _o1->m_pStrParam = "BIN_BOSS_BGANM_46_1_1"; return _o1; }(), []() { Constants::LAND_ATTACK_INFO* _o2 = new Constants::LAND_ATTACK_INFO(); _o2->m_nFrame = 0u; _o2->m_nKind = 2u; _o2->m_pUintParam = 3u; return _o2; }(), []() { Constants::LAND_ATTACK_INFO* _o3 = new Constants::LAND_ATTACK_INFO(); _o3->m_nFrame = 0u; _o3->m_nKind = 3u; return _o3; }(), []() { Constants::LAND_ATTACK_INFO* _o4 = new Constants::LAND_ATTACK_INFO(); _o4->m_nFrame = 0u; _o4->m_nKind = 4u; return _o4; }(), []() { Constants::LAND_ATTACK_INFO* _o5 = new Constants::LAND_ATTACK_INFO(); _o5->m_nFrame = 0u; _o5->m_nKind = 5u; return _o5; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind01[5] = {[]() { Constants::LAND_ATTACK_INFO* _o6 = new Constants::LAND_ATTACK_INFO(); _o6->m_nFrame = 0u; _o6->m_nKind = 1u; _o6->m_pStrParam = "BIN_BOSS_BGANM_46_1_2"; return _o6; }(), []() { Constants::LAND_ATTACK_INFO* _o7 = new Constants::LAND_ATTACK_INFO(); _o7->m_nFrame = 0u; _o7->m_nKind = 2u; _o7->m_pUintParam = 3u; return _o7; }(), []() { Constants::LAND_ATTACK_INFO* _o8 = new Constants::LAND_ATTACK_INFO(); _o8->m_nFrame = 0u; _o8->m_nKind = 3u; return _o8; }(), []() { Constants::LAND_ATTACK_INFO* _o9 = new Constants::LAND_ATTACK_INFO(); _o9->m_nFrame = 0u; _o9->m_nKind = 4u; return _o9; }(), []() { Constants::LAND_ATTACK_INFO* _o10 = new Constants::LAND_ATTACK_INFO(); _o10->m_nFrame = 0u; _o10->m_nKind = 5u; return _o10; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind02[5] = {[]() { Constants::LAND_ATTACK_INFO* _o11 = new Constants::LAND_ATTACK_INFO(); _o11->m_nFrame = 0u; _o11->m_nKind = 1u; _o11->m_pStrParam = "BIN_BOSS_BGANM_46_1_3"; return _o11; }(), []() { Constants::LAND_ATTACK_INFO* _o12 = new Constants::LAND_ATTACK_INFO(); _o12->m_nFrame = 0u; _o12->m_nKind = 2u; _o12->m_pUintParam = 2u; return _o12; }(), []() { Constants::LAND_ATTACK_INFO* _o13 = new Constants::LAND_ATTACK_INFO(); _o13->m_nFrame = 0u; _o13->m_nKind = 3u; return _o13; }(), []() { Constants::LAND_ATTACK_INFO* _o14 = new Constants::LAND_ATTACK_INFO(); _o14->m_nFrame = 0u; _o14->m_nKind = 4u; return _o14; }(), []() { Constants::LAND_ATTACK_INFO* _o15 = new Constants::LAND_ATTACK_INFO(); _o15->m_nFrame = 0u; _o15->m_nKind = 5u; return _o15; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind03[5] = {[]() { Constants::LAND_ATTACK_INFO* _o16 = new Constants::LAND_ATTACK_INFO(); _o16->m_nFrame = 0u; _o16->m_nKind = 1u; _o16->m_pStrParam = "BIN_BOSS_BGANM_46_1_4"; return _o16; }(), []() { Constants::LAND_ATTACK_INFO* _o17 = new Constants::LAND_ATTACK_INFO(); _o17->m_nFrame = 0u; _o17->m_nKind = 2u; _o17->m_pUintParam = 1u; return _o17; }(), []() { Constants::LAND_ATTACK_INFO* _o18 = new Constants::LAND_ATTACK_INFO(); _o18->m_nFrame = 0u; _o18->m_nKind = 3u; return _o18; }(), []() { Constants::LAND_ATTACK_INFO* _o19 = new Constants::LAND_ATTACK_INFO(); _o19->m_nFrame = 0u; _o19->m_nKind = 4u; return _o19; }(), []() { Constants::LAND_ATTACK_INFO* _o20 = new Constants::LAND_ATTACK_INFO(); _o20->m_nFrame = 0u; _o20->m_nKind = 5u; return _o20; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind04[6] = {[]() { Constants::LAND_ATTACK_INFO* _o21 = new Constants::LAND_ATTACK_INFO(); _o21->m_nFrame = 0u; _o21->m_nKind = 1u; _o21->m_pStrParam = "BIN_BOSS_BGANM_46_2_1"; return _o21; }(), []() { Constants::LAND_ATTACK_INFO* _o22 = new Constants::LAND_ATTACK_INFO(); _o22->m_nFrame = 0u; _o22->m_nKind = 2u; _o22->m_pUintParam = 2u; return _o22; }(), []() { Constants::LAND_ATTACK_INFO* _o23 = new Constants::LAND_ATTACK_INFO(); _o23->m_nFrame = 500u; _o23->m_nKind = 2u; _o23->m_pUintParam = 4u; return _o23; }(), []() { Constants::LAND_ATTACK_INFO* _o24 = new Constants::LAND_ATTACK_INFO(); _o24->m_nFrame = 0u; _o24->m_nKind = 3u; return _o24; }(), []() { Constants::LAND_ATTACK_INFO* _o25 = new Constants::LAND_ATTACK_INFO(); _o25->m_nFrame = 0u; _o25->m_nKind = 4u; return _o25; }(), []() { Constants::LAND_ATTACK_INFO* _o26 = new Constants::LAND_ATTACK_INFO(); _o26->m_nFrame = 0u; _o26->m_nKind = 5u; return _o26; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind05[6] = {[]() { Constants::LAND_ATTACK_INFO* _o27 = new Constants::LAND_ATTACK_INFO(); _o27->m_nFrame = 0u; _o27->m_nKind = 1u; _o27->m_pStrParam = "BIN_BOSS_BGANM_46_2_2"; return _o27; }(), []() { Constants::LAND_ATTACK_INFO* _o28 = new Constants::LAND_ATTACK_INFO(); _o28->m_nFrame = 0u; _o28->m_nKind = 2u; _o28->m_pUintParam = 3u; return _o28; }(), []() { Constants::LAND_ATTACK_INFO* _o29 = new Constants::LAND_ATTACK_INFO(); _o29->m_nFrame = 500u; _o29->m_nKind = 2u; _o29->m_pUintParam = 4u; return _o29; }(), []() { Constants::LAND_ATTACK_INFO* _o30 = new Constants::LAND_ATTACK_INFO(); _o30->m_nFrame = 0u; _o30->m_nKind = 3u; return _o30; }(), []() { Constants::LAND_ATTACK_INFO* _o31 = new Constants::LAND_ATTACK_INFO(); _o31->m_nFrame = 0u; _o31->m_nKind = 4u; return _o31; }(), []() { Constants::LAND_ATTACK_INFO* _o32 = new Constants::LAND_ATTACK_INFO(); _o32->m_nFrame = 0u; _o32->m_nKind = 5u; return _o32; }()};
const Constants::LAND_ATTACK_INFO* cTaskCharGolvis03::s_aKind06[6] = {[]() { Constants::LAND_ATTACK_INFO* _o33 = new Constants::LAND_ATTACK_INFO(); _o33->m_nFrame = 0u; _o33->m_nKind = 1u; _o33->m_pStrParam = "BIN_BOSS_BGANM_46_3_1"; return _o33; }(), []() { Constants::LAND_ATTACK_INFO* _o34 = new Constants::LAND_ATTACK_INFO(); _o34->m_nFrame = 0u; _o34->m_nKind = 2u; _o34->m_pUintParam = 3u; return _o34; }(), []() { Constants::LAND_ATTACK_INFO* _o35 = new Constants::LAND_ATTACK_INFO(); _o35->m_nFrame = 900u; _o35->m_nKind = 2u; _o35->m_pUintParam = 4u; return _o35; }(), []() { Constants::LAND_ATTACK_INFO* _o36 = new Constants::LAND_ATTACK_INFO(); _o36->m_nFrame = 0u; _o36->m_nKind = 3u; return _o36; }(), []() { Constants::LAND_ATTACK_INFO* _o37 = new Constants::LAND_ATTACK_INFO(); _o37->m_nFrame = 0u; _o37->m_nKind = 4u; return _o37; }(), []() { Constants::LAND_ATTACK_INFO* _o38 = new Constants::LAND_ATTACK_INFO(); _o38->m_nFrame = 0u; _o38->m_nKind = 5u; return _o38; }()};
const Constants::LAND_ATTACK_INFO* const* cTaskCharGolvis03::s_aaAttackTBL[7] = {s_aKind00, s_aKind01, s_aKind02, s_aKind03, s_aKind04, s_aKind05, s_aKind06};
const Constants::gmSGolvisAttack* cTaskCharGolvis03::s_AttackTBL[2] = {[]() { Constants::gmSGolvisAttack* _o39 = new Constants::gmSGolvisAttack(); _o39->uiKind = 39; _o39->fRangeMin = 0.0f; _o39->fRangeMax = 999.0f; _o39->iTimeMin = 0; _o39->iTimeMax = 5; return _o39; }(), []() { Constants::gmSGolvisAttack* _o40 = new Constants::gmSGolvisAttack(); _o40->uiKind = 0; return _o40; }()};
uint cTaskCharGolvis03::uiPlayMotionTable[5] = {};

cTaskCharGolvis03::cTaskCharGolvis03(cJetTask* pParent)
	: cTaskCharGolvis(pParent)
{
	SetName("cTaskCharGolvis03");
	Init();
}

// DOL 0x8011b5e0 [audited]
void cTaskCharGolvis03::Init()
{
	::Constants::jetGeoVecSet(vTrans(), 0.0f, 0.0f, -16.0f, 1.0f);
	Setting(false);
}

// DOL 0x8011b610 [audited]
void cTaskCharGolvis03::Term()
{
	// DOL blr: the base Term runs from the dtor chain (cTaskCharGolvis::ReleaseTerm)
}

// DOL 0x8011b614 [audited]
bool cTaskCharGolvis03::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss())
	{
		pHitChar(1)->FlagOff(17u);
		MonsterFrameMove();
	}
	return cTaskCharGolvis::Execute(pParam);
}

// DOL 0x8011b6bc [audited]
void cTaskCharGolvis03::Draw(int nStatus, cJetTask* pParam)
{
	cTaskCharGolvis::Draw(nStatus, pParam);
}

// DOL 0x8011b6c0 [audited: motion table 0/9/10/11, push-out level 2 vs disassembly]
void cTaskCharGolvis03::Setting(bool bRelocate)
{
	cTaskCharGolvis::Setting(bRelocate);
	uiPlayMotionTable[0] = 0u;
	uiPlayMotionTable[1] = 9u;
	uiPlayMotionTable[2] = 10u;
	uiPlayMotionTable[3] = 11u;
	nPushOutLevel() = 2u;
	m_iState = Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_PLAY;
	if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE075_ENSOU, false, pPBModel(nType())->pScnMdl()));
	AttackLandRegist(Constants::PACN_APP()->pTaskBgGimmick());
	pAttackLand()->SetAttackTBL(s_aaAttackTBL);
	m_iPlayMotion = Constants::gmEMonsterPlayMotionGolvisLand::PLAYMOTION_GOLVISLAND_NONE;
	InitStateFirst();
}

// DOL 0x8011b7a8 [audited]
void cTaskCharGolvis03::InitStateFirst()
{
	ChangeToAppear();
	m_bDisp = true;
}

// DOL 0x8011b7e8 [audited]
void cTaskCharGolvis03::InitStateStart()
{
	m_iAIMode = 2;
	m_iState = Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_PLAY;
	m_iPlayMotion = Constants::gmEMonsterPlayMotionGolvisLand::PLAYMOTION_GOLVISLAND_NONE;
}

// DOL 0x8011b800 [audited]
void cTaskCharGolvis03::MonsterFrameMove_PreProcess()
{
	uint pMes = 0u;
	if (AttackLandGetMsg(pMes))
	{
		m_iPlayMotion = (Constants::gmEMonsterPlayMotionGolvisLand)pMes;
	}
}

// DOL 0x8011b84c [audited: area bounds -4/4/-21/-14, 9 degrees, vtable slots (+0xc0 = ChangeToAttack) vs decomp]
void cTaskCharGolvis03::MonsterFrameMove_Free()
{
	uint anmState = GetAnmState();
	Vector4 v;
	::Constants::jetGeoVecCopy(v, Constants::PACN_APP()->pTaskCharPacman()->vTrans());
	(void)(m_iState);
	nPushOutLevel() = 2u;
	m_bEnableGravity = true;
	if (nType() != 2)
	{
		if (vTrans()[0] == m_vFirstTrans[0] && vTrans()[2] == m_vFirstTrans[2])
		{
			m_iState = Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_PLAY;
		}
		else
		{
			m_iState = Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_MOVE;
		}
	}
	else
	{
		m_iState = Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_ESCAPE;
	}
	switch (m_iState)
	{
	case Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_PLAY:
		{
			float pOut;
			::Constants::bizMathNearRad(pOut, vRotate()[1], Mathf::Atan2(v[0] - vTrans()[0], v[2] - vTrans()[2]), ::Constants::JET_DEG2RAD(9.0f));
			vRotate()[1] = pOut;
			if (m_iPlayMotion == Constants::gmEMonsterPlayMotionGolvisLand::PLAYMOTION_GOLVISLAND_FAINT)
			{
				ChangeToAttack(14);
				m_iPlayMotion = Constants::gmEMonsterPlayMotionGolvisLand::PLAYMOTION_GOLVISLAND_NONE;
			}
			else if (anmState != uiPlayMotionTable[(int)m_iPlayMotion])
			{
				anmState = uiPlayMotionTable[(int)m_iPlayMotion];
				SetAnmState(anmState, 10u);
			}
			else if (TryChangeToLaugh())
			{
				break;
			}
			if (AttackLandIsEnd())
			{
				TryChangeToAttack();
			}
			break;
		}
	case Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_MOVE:
	case Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_ESCAPE:
		{
			if (m_iState == Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_MOVE)
			{
				::Constants::bizMathNearTransXZ(vTrans(), vTrans(), m_vFirstTrans, m_fMoveSpeed);
				break;
			}
			Vector4 v2;
			::Constants::jetGeoVecSub(v2, vTrans(), v);
			Vector4 v3;
			::Constants::jetGeoVecAdd(v3, v2, vTrans());
			if (v3[0] < -4.0f && CustomKingdomUtil::IF(v2[0]))
			{
				float num = (-4.0f - vTrans()[0]) / v2[0];
				v2[0] *= num;
			}
			if (v3[0] > 4.0f && CustomKingdomUtil::IF(v2[0]))
			{
				float num = (4.0f - vTrans()[0]) / v2[0];
				v2[0] *= num;
			}
			if (v3[2] < -21.0f && CustomKingdomUtil::IF(v2[2]))
			{
				float num = (-21.0f - vTrans()[2]) / v2[2];
				v2[2] *= num;
			}
			if (v3[2] > -14.0f && CustomKingdomUtil::IF(v2[2]))
			{
				float num = (-14.0f - vTrans()[2]) / v2[2];
				v2[2] *= num;
			}
			::Constants::jetGeoVecAdd(v3, v2, vTrans());
			::Constants::bizMathNearTransXZ(vTrans(), vTrans(), v3, m_fBijikeMoveSpeed);
			break;
		}
	}
	if (m_iState == Constants::gmEMonsterStateGolvisLand::STATE_GOLVISLAND_PLAY)
	{
		if (!SoundManager::get_SE()->IsPlayinSEKind(ESEKind::PACN_ROLL_SND_SE075_ENSOU))
		{
			if (cPacnRollLoopSndMan::CanPlay()) cPacnRollLoopSndMan::Track(SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE075_ENSOU, false, pPBModel(nType())->pScnMdl()));
		}
	}
	else
	{
		SoundManager::get_SE()->Stop(ESEKind::PACN_ROLL_SND_SE075_ENSOU);
	}
}

// DOL 0x8011bb94 [audited]
void cTaskCharGolvis03::MonsterFrameMove_Escape()
{
	MonsterFrameMove_Free();
}

// DOL 0x8011bba4 [audited]
Constants::gmSGolvisAttack* cTaskCharGolvis03::GetAttackKind(uint nEatNum, uint nIjikeLevel, int nNum)
{
	return cTaskCharGolvis::GetAttackKind(nEatNum, nIjikeLevel, nNum);
}

}
