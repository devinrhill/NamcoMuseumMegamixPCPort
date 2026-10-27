#include "kingdom/game/g07/cTaskMap.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g07/cMotosMapoo.hpp"
#include "kingdom/game/g07/Constants.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "kingdom/game/g07/cTaskCharEnemy.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBall.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBeecon.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBeetle.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss01.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss02.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss03.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss04.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss05.hpp"
#include "kingdom/game/g07/cTaskCharEnemyBoss05Last.hpp"
#include "kingdom/game/g07/cTaskCharEnemyFireBee.hpp"
#include "kingdom/game/g07/cTaskCharEnemyFireBug.hpp"
#include "kingdom/game/g07/cTaskCharEnemyGiga.hpp"
#include "kingdom/game/g07/cTaskCharEnemyLadyBug.hpp"
#include "kingdom/game/g07/cTaskCharEnemyMega.hpp"
#include "kingdom/game/g07/cTaskCharEnemyNabicon.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPeta.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPolar.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaBlack.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaBlue.hpp"
#include "kingdom/game/g07/cTaskCharEnemyPupaRed.hpp"
#include "kingdom/game/g07/cTaskCharEnemySpirus.hpp"
#include "kingdom/game/g07/cTaskCharEnemyTaitorian.hpp"
#include "kingdom/game/g07/cTaskCharItem.hpp"
#include "kingdom/game/g07/cTaskCharItemCharge.hpp"
#include "kingdom/game/g07/cTaskCharItemJump.hpp"
#include "kingdom/game/g07/cTaskCharItemPower.hpp"
#include "kingdom/game/g07/cTaskCharPlayer.hpp"
#include "kingdom/game/g07/cTaskGimmickBase.hpp"
#include "kingdom/game/g07/cTaskGimmickHatch.hpp"
#include "kingdom/game/g07/cTaskGimmickReaction.hpp"
#include "kingdom/game/g07/cTaskGimmickSwitch.hpp"
#include "kingdom/game/g07/cTaskPanelMan.hpp"
#include "kingdom/game/g07/MAP_HIT_RESULT.hpp"
#include "kingdom/game/g07/sMOTOS_POS2D.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>

namespace kingdom::game::g07
{

const cTaskMap::ENEMY_TASK_KIND_TBL cTaskMap::s_enemy_task_tbl[27] = {nullptr, &cTaskCharEnemyPupaRed::Construct, &cTaskCharEnemyPupaBlue::Construct, &cTaskCharEnemyPupaBlack::Construct, &cTaskCharEnemyPolar::Construct, &cTaskCharEnemySpirus::Construct, &cTaskCharEnemyFireBug::Construct, &cTaskCharEnemyFireBee::Construct, &cTaskCharEnemyTaitorian::Construct, &cTaskCharEnemyLadyBug::Construct, &cTaskCharEnemyBeetle::Construct, &cTaskCharEnemyMega::Construct, &cTaskCharEnemyGiga::Construct, nullptr, &cTaskCharEnemyBeecon::Construct, &cTaskCharEnemyNabicon::Construct, &cTaskCharEnemyBoss01::Construct, &cTaskCharEnemyBoss02::Construct, &cTaskCharEnemyBoss03::Construct, &cTaskCharEnemyBoss04::Construct, &cTaskCharEnemyBoss05::Construct, nullptr, &cTaskCharEnemyBall::Construct, &cTaskCharEnemyBall::Construct, &cTaskCharEnemyPeta::Construct, nullptr, &cTaskCharEnemyBoss05Last::Construct};
const cTaskMap::ITEM_TASK_KIND_TBL cTaskMap::s_item_task_tbl[4] = {nullptr, &cTaskCharItemPower::Construct, &cTaskCharItemJump::Construct, &cTaskCharItemCharge::Construct};
const cTaskMap::GIMMICK_TASK_KIND_TBL cTaskMap::s_gimmick_task_tbl[4] = {nullptr, &cTaskGimmickReaction::Construct, &cTaskGimmickHatch::Construct, &cTaskGimmickSwitch::Construct};

cTaskMap::cTaskMap()
{
	Init();
}

cTaskCharEnemy*& cTaskMap::ppStackEnemy(uint set)
{
	CustomKingdomUtil::Assert(set < 16, "error");
	return m_pStackEnemy[set];
}

cTaskPanelMan* cTaskMap::pTaskPanelMan()
{
	return m_pTaskPanelMan;
}

MAP_HIT_RESULT* cTaskMap::HitResult(int n)
{
	CustomKingdomUtil::Assert(m_pTaskPanelMan != nullptr, "nullpo");
	return m_pTaskPanelMan->HitResult(n);
}

cTaskGimmickBase* cTaskMap::GetGimmickArray(uint num)
{
	CustomKingdomUtil::Assert(num < 256, "muripo");
	return m_apGimmicks[num];
}

cTaskGimmickBase* cTaskMap::GetGimmickArray(uint x, uint z)
{
	CustomKingdomUtil::Assert(x + z * nWidth() < 256, "muripo");
	return m_apGimmicks[x + z * nWidth()];
}

void cTaskMap::SetGimmickArray(uint num, cTaskGimmickBase* p)
{
	CustomKingdomUtil::Assert(num < 256, "muripo");
	m_apGimmicks[num] = p;
}

cTaskGimmickBase* cTaskMap::pGimmicks(uint num)
{
	CustomKingdomUtil::Assert(num < 32, "muripo");
	return m_pGimmicks[num];
}

void cTaskMap::pGimmicks(uint num, cTaskGimmickBase* p)
{
	CustomKingdomUtil::Assert(num < 32, "muripo");
	m_pGimmicks[num] = p;
}

void cTaskMap::Init()
{
	Mapoo = new cMotosMapoo();
	CustomKingdomUtil::Assert(KingdomGlobal::g_pMotosGame != nullptr, "error!");
	KingdomGlobal::g_pMotosGame->pSetMap(this);
	for (int i = 0; i < 256; i++)
	{
		m_apGimmicks[i] = nullptr;
	}
	for (int j = 0; j < 32; j++)
	{
		m_pGimmicks[j] = nullptr;
	}
	for (int k = 0; k < (int)std::size(m_pStackEnemy); k++)
	{
		m_pStackEnemy[k] = nullptr;
	}
	KingdomGlobal::g_pMotosGame->bGimmick() = false;
	ConnectChild(m_pTaskPanelMan = new cTaskPanelMan(KingdomGlobal::g_pMotosGame));
	SetupPlayer();
	SetupEnemy();
	SetupItem();
	SetupGimmick();
}

void cTaskMap::Term()
{
	m_pTaskPanelMan->TermOnce();
	for (int i = 0; i < (int)std::size(m_apGimmicks); i++)
	{
		if (m_apGimmicks[i] != nullptr)
		{
			m_apGimmicks[i]->TermOnce();
		}
	}
	for (int j = 0; j < (int)std::size(m_pStackEnemy); j++)
	{
		if (m_pGimmicks[j] != nullptr)
		{
			m_pGimmicks[j]->TermOnce();
		}
	}
}

void cTaskMap::SetupPlayer()
{
	Mapoo->SearchObj(524288u, cTaskMap::SetupPlayerCB, this);
}

bool cTaskMap::SetupPlayerCB(DObjParam::sObj* obj, cJetTask* args)
{
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	cTaskMap* cTaskMap2 = static_cast<cTaskMap*>(args);
	uint num = (obj->m_type & 0xF) - 1;
	float num2 = cTaskMap2->fPanelSize();
	if (KingdomGlobal::g_pMotosGame->bMultiPlay() && num >= KingdomGlobal::MotosSys->pGetMatchingData()->nNum)
	{
		return true;
	}
	cTaskCharPlayer* cTaskCharPlayer2 = new cTaskCharPlayer();
	if (cTaskCharPlayer2 == nullptr)
	{
		CustomKingdomUtil::Assert(cTaskCharPlayer2 != nullptr, "error");
	}
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	switch (obj->m_param[0])
	{
	case 0:
		v[1] = ::Constants::JET_DEG2RAD(180.0f);
		break;
	case 1:
		v[1] = ::Constants::JET_DEG2RAD(0.0f);
		break;
	case 2:
		v[1] = ::Constants::JET_DEG2RAD(270.0f);
		break;
	case 3:
		v[1] = ::Constants::JET_DEG2RAD(90.0f);
		break;
	}
	::Constants::jetGeoVecCopy(cTaskCharPlayer2->vRotate(), v);
	cTaskCharPlayer2->vMTrans()[0] = (float)obj->m_pos[0] * num2 / 16.0f;
	cTaskCharPlayer2->vMTrans()[1] = 0.0f;
	cTaskCharPlayer2->vMTrans()[2] = (float)obj->m_pos[1] * num2 / 16.0f;
	::Constants::jetGeoVecCopy(cTaskCharPlayer2->vMBegin(), cTaskCharPlayer2->vMTrans());
	KingdomGlobal::g_pMotosGame->RegistPlayers(cTaskCharPlayer2);
	cTaskMap2->ConnectChild(cTaskCharPlayer2);
	return true;
}

void cTaskMap::SetupEnemy()
{
	Mapoo->SearchObj(1048576u, cTaskMap::SetupEnemyCB, this);
	SetFormation();
	for (uint num = 0u; num < 16; num++)
	{
		if (ppStackEnemy(num) != nullptr)
		{
			ConnectChild(ppStackEnemy(num));
		}
	}
}

bool cTaskMap::SetupEnemyFormCB(DObjParam::sObj* obj, cJetTask* args)
{
	cTaskCharEnemy* cTaskCharEnemy2 = static_cast<cTaskCharEnemy*>(args);
	int num = obj->m_param[0];
	if (cTaskCharEnemy2->nFormGroup() == num)
	{
		int nNo = obj->m_param[1];
		int nMeetTime = obj->m_param[2];
		int nMoveTime = obj->m_param[3];
		int num2 = cTaskCharEnemy2->nFormNum();
		int num3 = -1;
		std::vector<Constants::FORM_GROUP_INFO*>& array = cTaskCharEnemy2->pFormGroupInfo();
		Constants::FORM_GROUP_INFO* fORM_GROUP_INFO;
		for (int i = 0; i < num2; i++)
		{
			fORM_GROUP_INFO = array[i];
			if (fORM_GROUP_INFO->m_nType == 0)
			{
				fORM_GROUP_INFO->m_nType = (int)obj->m_type;
				fORM_GROUP_INFO->m_nNo = nNo;
				fORM_GROUP_INFO->m_nMeetTime = nMeetTime;
				fORM_GROUP_INFO->m_nMoveTime = nMoveTime;
				num3 = i;
				break;
			}
		}
		if (num3 == -1)
		{
			return false;
		}
		Constants::FORM_GROUP_INFO* fORM_GROUP_INFO2 = array[num3];
		if (num3 == 0)
		{
			cTaskCharEnemy2->pFormNow() = fORM_GROUP_INFO2;
			return true;
		}
		fORM_GROUP_INFO = cTaskCharEnemy2->pFormNow();
		if (fORM_GROUP_INFO2->m_nNo < fORM_GROUP_INFO->m_nNo)
		{
			fORM_GROUP_INFO2->m_pNext = fORM_GROUP_INFO;
			cTaskCharEnemy2->pFormNow() = fORM_GROUP_INFO2;
		}
		else
		{
			while (fORM_GROUP_INFO != nullptr)
			{
				Constants::FORM_GROUP_INFO* pNext = fORM_GROUP_INFO->m_pNext;
				if (pNext == nullptr || fORM_GROUP_INFO2->m_nNo < pNext->m_nNo)
				{
					fORM_GROUP_INFO2->m_pNext = pNext;
					fORM_GROUP_INFO->m_pNext = fORM_GROUP_INFO2;
					break;
				}
				fORM_GROUP_INFO = pNext;
			}
		}
	}
	return true;
}

void cTaskMap::SetFormation()
{
	for (uint num = 0u; num < 32; num++)
	{
		if (KingdomGlobal::g_pMotosGame->pEnemys(num) == nullptr || KingdomGlobal::g_pMotosGame->pEnemys(num)->nFormPart() != 1)
		{
			continue;
		}
		cTaskCharEnemy* cTaskCharEnemy2 = KingdomGlobal::g_pMotosGame->pEnemys(num);
		cTaskCharEnemy* cTaskCharEnemy3 = KingdomGlobal::g_pMotosGame->pEnemys(num);
		cTaskCharEnemy* cTaskCharEnemy4 = nullptr;
		int num2 = (cTaskCharEnemy2->nFormNum() = Mapoo->SearchFormNum(cTaskCharEnemy2->nFormGroup()));
		if (num2 != 0)
		{
			std::vector<Constants::FORM_GROUP_INFO*> array(num2);
			for (int i = 0; i < (int)array.size(); i++)
			{
				array[i] = new Constants::FORM_GROUP_INFO();
			}
			cTaskCharEnemy2->pFormGroupInfo() = array;
			if (!Mapoo->SearchObj(4194304u, cTaskMap::SetupEnemyFormCB, cTaskCharEnemy2))
			{
				cTaskCharEnemy2->nFormPart() = 0;
				continue;
			}
			for (uint num3 = 0u; num3 < 32; num3++)
			{
				cTaskCharEnemy4 = KingdomGlobal::g_pMotosGame->pEnemys(num3);
				if (cTaskCharEnemy4 != nullptr && cTaskCharEnemy4 != cTaskCharEnemy2 && cTaskCharEnemy4->nFormPart() == 2 && cTaskCharEnemy4->nFormGroup() == cTaskCharEnemy2->nFormGroup())
				{
					cTaskCharEnemy4->pFormInfo()->m_pParent = cTaskCharEnemy2;
					cTaskCharEnemy4->pFormInfo()->m_pChild = nullptr;
					cTaskCharEnemy3->pFormInfo()->m_pChild = cTaskCharEnemy4;
					cTaskCharEnemy3 = cTaskCharEnemy4;
				}
			}
		}
		else
		{
			cTaskCharEnemy2->nFormPart() = 0;
		}
	}
}

bool cTaskMap::SetupEnemyCB(DObjParam::sObj* obj, cJetTask* args)
{
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	cTaskMap* cTaskMap2 = static_cast<cTaskMap*>(args);
	uint num = (uint)((long)obj->m_type & (long)(-1048577));
	uint num2 = (uint)::Constants::ARRAY_SIZE(s_enemy_task_tbl);
	if (num == 0)
	{
		return true;
	}
	if (num > num2)
	{
		return true;
	}
	if (s_enemy_task_tbl[num] == nullptr)
	{
		return true;
	}
	float num3 = cTaskMap2->fPanelSize();
	cTaskCharEnemy* cTaskCharEnemy2 = nullptr;
	if (!KingdomGlobal::g_pMotosGame->IsFirstPlay() && KingdomGlobal::MotosSys->nLastBossType() == 1 && num == 20)
	{
		num = 26u;
	}
	cTaskCharEnemy2 = s_enemy_task_tbl[num]();
	if (cTaskCharEnemy2 == nullptr)
	{
		CustomKingdomUtil::Assert(cTaskCharEnemy2 != nullptr, "error");
		return true;
	}
	cTaskCharEnemy2->Create(num);
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	switch (obj->m_param[0])
	{
	case 0:
		v[1] = ::Constants::JET_DEG2RAD(180.0f);
		break;
	case 1:
		v[1] = ::Constants::JET_DEG2RAD(0.0f);
		break;
	case 2:
		v[1] = ::Constants::JET_DEG2RAD(270.0f);
		break;
	case 3:
		v[1] = ::Constants::JET_DEG2RAD(90.0f);
		break;
	}
	::Constants::jetGeoVecCopy(cTaskCharEnemy2->vRotate(), v);
	cTaskCharEnemy2->InitParam(Constants::get_g_MoEnemyParam()[num]);
	cTaskCharEnemy2->nChipColor() = cTaskCharEnemy2->Kind2ChipColor((int)num);
	cTaskCharEnemy2->vMTrans()[0] = (float)obj->m_pos[0] * num3 / 16.0f;
	cTaskCharEnemy2->vMTrans()[1] = 0.0f;
	cTaskCharEnemy2->vMTrans()[2] = (float)obj->m_pos[1] * num3 / 16.0f;
	cTaskCharEnemy2->nKind() = num;
	cTaskCharEnemy2->SetMMSParam(obj->m_param[1], obj->m_param[2], obj->m_param[3]);
	if (cTaskCharEnemy2->nFormPart() != 0)
	{
		if (cTaskCharEnemy2->nFormPart() == 2)
		{
			uint num4 = 15u;
			while (cTaskMap2->ppStackEnemy(num4) != nullptr)
			{
				num4--;
			}
			cTaskMap2->ppStackEnemy(num4) = cTaskCharEnemy2;
		}
		else
		{
			uint num5;
			for (num5 = 0u; cTaskMap2->ppStackEnemy(num5) != nullptr; num5++)
			{
			}
			cTaskMap2->ppStackEnemy(num5) = cTaskCharEnemy2;
		}
		cTaskCharEnemy2->SetMode(2);
	}
	else
	{
		cTaskMap2->ConnectChild(cTaskCharEnemy2);
	}
	::Constants::jetGeoVecCopy(cTaskCharEnemy2->vMBeginTrans(), cTaskCharEnemy2->vMTrans());
	cTaskMap2->Map2World(cTaskCharEnemy2->vWTrans(), cTaskCharEnemy2->vMTrans());
	if (KingdomGlobal::MotosSys->bMotosCharDead(cTaskCharEnemy2->GrobalID()))
	{
		cTaskCharEnemy2->bForceDeath() = true;
	}
	return true;
}

void cTaskMap::SetupItem()
{
	Mapoo->SearchObj(262144u, cTaskMap::SetupItemCB, this);
}

bool cTaskMap::SetupItemCB(DObjParam::sObj* obj, cJetTask* args)
{
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	cTaskMap* cTaskMap2 = static_cast<cTaskMap*>(args);
	uint num = (uint)((long)obj->m_type & (long)(-262145));
	uint num2 = (uint)::Constants::ARRAY_SIZE(s_item_task_tbl);
	if (num == 0)
	{
		return true;
	}
	if (num >= num2)
	{
		return true;
	}
	float num3 = cTaskMap2->fPanelSize();
	cTaskCharItem* cTaskCharItem2 = nullptr;
	cTaskCharItem2 = s_item_task_tbl[num]();
	if (cTaskCharItem2 == nullptr)
	{
		CustomKingdomUtil::Assert(cTaskCharItem2 != nullptr, "error");
		return true;
	}
	cTaskCharItem2->Create(0u);
	cTaskCharItem2->Setting((int)num);
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	v[1] = ::Constants::JET_DEG2RAD(0.0f);
	::Constants::jetGeoVecCopy(cTaskCharItem2->vRotate(), v);
	cTaskCharItem2->vMTrans()[0] = (float)obj->m_pos[0] * num3 / 16.0f;
	cTaskCharItem2->vMTrans()[1] = 0.0f;
	cTaskCharItem2->vMTrans()[2] = (float)obj->m_pos[1] * num3 / 16.0f;
	cTaskMap2->Map2World(cTaskCharItem2->vWTrans(), cTaskCharItem2->vMTrans());
	cTaskMap2->ConnectChild(cTaskCharItem2);
	if (KingdomGlobal::MotosSys->bMotosCharDead(cTaskCharItem2->GrobalID()))
	{
		cTaskCharItem2->bForceDeath() = true;
	}
	return true;
}

void cTaskMap::SetupGimmick()
{
	Mapoo->SearchObj(131072u, cTaskMap::SetupGimmickCB, this);
}

bool cTaskMap::SetupGimmickCB(DObjParam::sObj* obj, cJetTask* args)
{
	CustomKingdomUtil::Assert(args != nullptr, "nullpo");
	cTaskMap* cTaskMap2 = static_cast<cTaskMap*>(args);
	uint num = (uint)((long)obj->m_type & (long)(-131073));
	uint num2 = (uint)::Constants::ARRAY_SIZE(s_gimmick_task_tbl);
	if (num == 5 || num == 6)
	{
		num = 1u;
	}
	if (num == 0)
	{
		return true;
	}
	if (num >= num2)
	{
		return true;
	}
	float num3 = cTaskMap2->fPanelSize();
	cTaskGimmickBase* cTaskGimmickBase2 = nullptr;
	cTaskGimmickBase2 = s_gimmick_task_tbl[num]();
	if (cTaskGimmickBase2 == nullptr)
	{
		CustomKingdomUtil::Assert(cTaskGimmickBase2 != nullptr, "error");
		return true;
	}
	sMOTOS_POS2D array[7] = {[&]() { sMOTOS_POS2D _o10 = sMOTOS_POS2D(); _o10.pos[0] = 1; _o10.pos[1] = 1; return _o10; }(), [&]() { sMOTOS_POS2D _o11 = sMOTOS_POS2D(); _o11.pos[0] = 1; _o11.pos[1] = 1; return _o11; }(), [&]() { sMOTOS_POS2D _o12 = sMOTOS_POS2D(); _o12.pos[0] = 4; _o12.pos[1] = 4; return _o12; }(), [&]() { sMOTOS_POS2D _o13 = sMOTOS_POS2D(); _o13.pos[0] = 1; _o13.pos[1] = 1; return _o13; }(), [&]() { sMOTOS_POS2D _o14 = sMOTOS_POS2D(); _o14.pos[0] = 1; _o14.pos[1] = 1; return _o14; }(), [&]() { sMOTOS_POS2D _o15 = sMOTOS_POS2D(); _o15.pos[0] = 1; _o15.pos[1] = 1; return _o15; }(), [&]() { sMOTOS_POS2D _o16 = sMOTOS_POS2D(); _o16.pos[0] = 1; _o16.pos[1] = 1; return _o16; }()};
	cTaskGimmickBase2->nGmkWidth() = array[num].pos[0];
	cTaskGimmickBase2->nGmkDepth() = array[num].pos[1];
	cTaskGimmickBase2->Create(0u);
	uint num4 = cTaskMap2->nWidth();
	for (uint num5 = 0u; (long)num5 < (long)cTaskGimmickBase2->nGmkDepth(); num5++)
	{
		for (uint num6 = 0u; (long)num6 < (long)cTaskGimmickBase2->nGmkWidth(); num6++)
		{
			int num7 = (int)((float)obj->m_pos[0] / 16.0f + (float)num6 - (float)(cTaskGimmickBase2->nGmkWidth() / 2));
			int num8 = (int)((float)obj->m_pos[1] / 16.0f + (float)num5 - (float)(cTaskGimmickBase2->nGmkDepth() / 2));
			uint num9 = (uint)(num7 + (long)num8 * (long)num4);
			CustomKingdomUtil::Assert(cTaskMap2->GetGimmickArray(num9) == nullptr, "muripo!!!");
			cTaskMap2->SetGimmickArray(num9, cTaskGimmickBase2);
		}
	}
	for (uint num10 = 0u; num10 != 32; num10++)
	{
		if (cTaskMap2->pGimmicks(num10) == nullptr)
		{
			cTaskMap2->pGimmicks(num10, cTaskGimmickBase2);
			break;
		}
	}
	Vector4 v;
	::Constants::jetGeoVecZero(v);
	switch (obj->m_param[0])
	{
	case 0:
		v[1] = ::Constants::JET_DEG2RAD(180.0f);
		break;
	case 1:
		v[1] = ::Constants::JET_DEG2RAD(0.0f);
		break;
	case 2:
		v[1] = ::Constants::JET_DEG2RAD(270.0f);
		break;
	case 3:
		v[1] = ::Constants::JET_DEG2RAD(90.0f);
		break;
	}
	::Constants::jetGeoVecCopy(cTaskGimmickBase2->vRotate(), v);
	::Constants::jetGeoVecCopy(cTaskGimmickBase2->vRot(), v);
	cTaskGimmickBase2->nTaskKind() = num;
	cTaskGimmickBase2->vMTrans()[0] = (float)obj->m_pos[0] * num3 / 16.0f;
	cTaskGimmickBase2->vMTrans()[1] = 0.0f;
	cTaskGimmickBase2->vMTrans()[2] = (float)obj->m_pos[1] * num3 / 16.0f;
	cTaskGimmickBase2->nMapPos(0u) = (uint)obj->m_pos[0];
	cTaskGimmickBase2->nMapPos(1u) = (uint)obj->m_pos[1];
	for (int i = 0; i < 4; i++)
	{
		cTaskGimmickBase2->nWork(i) = (uint)obj->m_param[i];
	}
	cTaskMap2->Map2World(cTaskGimmickBase2->vWTrans(), cTaskGimmickBase2->vMTrans());
	cTaskMap2->ConnectChild(cTaskGimmickBase2);
	KingdomGlobal::g_pMotosGame->bGimmick() = true;
	return true;
}

bool cTaskMap::Execute(cJetTask* pParam)
{
	return true;
}

void cTaskMap::Draw(int nStatus, cJetTask* pParam)
{
}

void cTaskMap::Map2World(Vector4& vWorld, const Vector4& vMap)
{
	::Constants::jetGeoVecCopy(vWorld, vMap);
}

void cTaskMap::World2Map(Vector4& vMap, const Vector4& vWorld)
{
	::Constants::jetGeoVecCopy(vMap, vWorld);
}

uint cTaskMap::nWidth()
{
	return m_pTaskPanelMan->nWidth();
}

uint cTaskMap::nDepth()
{
	return m_pTaskPanelMan->nDepth();
}

float cTaskMap::fPanelSize()
{
	return m_pTaskPanelMan->fPanelSize();
}

int cTaskMap::HitCheck(Vector4 pPos, Vector4 pVec, float fRange)
{
	return m_pTaskPanelMan->HitCheck(pPos, pVec, fRange);
}

cTaskGimmickBase* cTaskMap::SearchGimmick(float mx, float mz)
{
	CustomKingdomUtil::Assert(fPanelSize() != 0.0f, "muripo");
	int x = (int)Mathf::Floor(mx / fPanelSize());
	int z = (int)Mathf::Floor(mz / fPanelSize());
	return SearchGimmickNo(x, z);
}

cTaskGimmickBase* cTaskMap::SearchGimmickNo(int x, int z)
{
	if (x < 0 || (long)x >= (long)nWidth() || z < 0 || (long)z >= (long)nDepth())
	{
		return nullptr;
	}
	if ((long)((uint)(x + (long)z * (long)nWidth())) >= (long)::Constants::ARRAY_SIZE(m_apGimmicks))
	{
		return nullptr;
	}
	return m_apGimmicks[x + (long)z * (long)nWidth()];
}

cTaskGimmickBase* cTaskMap::StepOnCnt(cTaskChar* pChar)
{
	Vector4 vWorld;
	Map2World(vWorld, pChar->vMTrans());
	return StepOnCntW(vWorld);
}

cTaskGimmickBase* cTaskMap::StepOnCntM(Vector4 mtrans)
{
	Vector4 vWorld;
	Map2World(vWorld, mtrans);
	return StepOnCntW(vWorld);
}

cTaskGimmickBase* cTaskMap::StepOnCntW(Vector4 wtrans)
{
	for (int i = 0; i < 32; i++)
	{
		cTaskGimmickBase* cTaskGimmickBase2 = m_pGimmicks[i];
		if (cTaskGimmickBase2 != nullptr && cTaskGimmickBase2->IsStepOnCntW(wtrans))
		{
			return cTaskGimmickBase2;
		}
	}
	return nullptr;
}

}
