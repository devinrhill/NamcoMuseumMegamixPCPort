#include "kingdom/game/g03/cTaskItem.hpp"

#include "platform/devtools.hpp"

#include <iterator>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/g03/cAppG03PacnRollGame.hpp"
#include "kingdom/game/g03/cG03PacnRollSys.hpp"
#include "kingdom/game/g03/cItem.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "kingdom/game/g03/cTaskBgBox.hpp"
#include "kingdom/game/g03/cTaskCharGhost.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"
#include "kingdom/game/g03/sSaveDataItem.hpp"
#include "kingdom/sKingdomData.hpp"
#include "KingdomGlobal.hpp"
#include <Constants.hpp>
#include <ESEKind.hpp>
#include <ReadFileUtil.hpp>
#include <SoundManager.hpp>
#include <SSaveDataPacnRoll.hpp>

namespace kingdom::game::g03
{

string cTaskItem::s_aBossCookie[3] = {"BIN_BOSS_COOKIE_SET_LIFE3_DAT", "BIN_BOSS_COOKIE_SET_LIFE2_DAT", "BIN_BOSS_COOKIE_SET_LIFE1_DAT"};

uint cTaskItem::GetCookieMax()
{
	return m_nCookieMax;
}

uint cTaskItem::GetAteCookie()
{
	return m_nAteCookie;
}

uint& cTaskItem::nHaveCookie()
{
	return m_nHaveCookie;
}

std::vector<cItem*>& cTaskItem::pVecItem()
{
	return m_vecItem;
}

bool cTaskItem::IsEatPizza()
{
	return m_bEatPizza;
}

cTaskItem::cTaskItem()
{
	SetName("cTaskItem");
	Init();
}

// DOL 0x80181bc4 [audited: described by the 43rd-session pass] (members +0x1cc cookie max, +0x1d0 ate, +0x1d4 have, +0x1d8 pizza eaten, +0x1dc the yahoo sound handle,
// +0x1e8 the item vector (0x40-byte records), +0x1f4 the three boss cookie index vectors). Retry branch: sys +7 (IsRetry) and not
// 0x8012a8a8 (IsBossStage: rounds 8, 13, 19, 24, 25)
void cTaskItem::Init()
{
	m_nCookieMax = 0u;
	m_nAteCookie = 0u;
	m_nHaveCookie = devtools::G03Cookies() >= 0 ? (uint)devtools::G03Cookies() : 0u;   // devtools: --g03-cookies (not 1:1)
	m_bEatPizza = false;
	SSaveDataPacnRoll::sStageRecord _out = SSaveDataPacnRoll::sStageRecord();
	if (!KingdomGlobal::PacnRollSys->IsTopMenuMode())
	{
		KingdomGlobal::PacnRollSys->pSaveLoadMan()->GetStageRecord(_out, KingdomGlobal::PacnRollSys->nRound());
		m_bEatPizza = _out.bEatPizza;
	}
	if (!KingdomGlobal::PacnRollSys->IsRetry() || KingdomGlobal::PacnRollSys->IsBossStage())
	{
		sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx("BIN_ITEM_SET_DAT");
		if (sKingdomData2 != nullptr && 0 < sKingdomData2->nSize)
		{
			std::vector<sSaveDataItem*> outParams;
			int outParamNum;
			int _discard1;
			ReadFileUtil::Convert<sSaveDataItem>(sKingdomData2->pBin, outParams, outParamNum, _discard1);
			for (int i = 0; i < outParamNum; i++)
			{
				cItem* cItem2 = new cItem();
				bool isUpdateFrame = outParams[i]->kind == 1;
				CreateModel(i, isUpdateFrame);
				cTaskModelEx::cPBModel* cPBModel = pPBModel(i);
				cPBModel->RegistModel(Kind2Key(outParams[i]->kind));
				cPBModel->IsDrawShadowDepth(true);
				cPBModel->SetLightSetIdx(1);
				cPBModel->SetFogIdx(0);
				cPBModel->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
				Visible(cPBModel);
				::Constants::jetGeoVecCopy(cPBModel->vTrans(), outParams[i]->trans);
				::Constants::jetGeoVecCopy(cItem2->vInitTrans(), outParams[i]->trans);
				if (1 == outParams[i]->kind)
				{
					cPBModel->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
				}
				cItem2->nKind() = outParams[i]->kind;
				cItem2->nDFlag() = 0u;
				cItem2->pPBModel() = cPBModel;
				cItem2->nRevivalCnt() = 0u;
				uint kind = outParams[i]->kind;
				if (kind - 2 <= 2)
				{
					cItem2->nRevivalCnt() = 120u;
				}
				if (cItem2->nKind() == 17 && m_bEatPizza)
				{
					cItem2->nDFlag() = 1u;
					Invisible(cPBModel);
				}
				if (cItem2->nKind() == 0 || 1 == cItem2->nKind())
				{
					m_nCookieMax++;
				}
				cPBModel->UpdateMatrix();
				UpdateTrans(cPBModel);
				cPBModel->UpdateShadowModelMtx();
				PushBackItem(cItem2);
			}
		}
	}
	else
	{
		int num = 0;
		m_nAteCookie = KingdomGlobal::PacnRollSys->pRetryData()->nAteCookie;
		m_nHaveCookie = KingdomGlobal::PacnRollSys->pRetryData()->nHaveCookie;
		for (cG03PacnRollSys::sRetryItem* item : KingdomGlobal::PacnRollSys->pRetryData()->vecItem)
		{
			cItem* cItem3 = new cItem();
			bool isUpdateFrame2 = item->nKind == 1;
			CreateModel(num, isUpdateFrame2);
			cTaskModelEx::cPBModel* cPBModel2 = pPBModel(num);
			cPBModel2->RegistModel(Kind2Key(item->nKind));
			cPBModel2->IsDrawShadowDepth(true);
			cPBModel2->SetLightSetIdx(1);
			cPBModel2->SetFogIdx(0);
			cPBModel2->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
			Visible(cPBModel2);
			::Constants::jetGeoVecCopy(cPBModel2->vTrans(), item->vInitTrans);
			::Constants::jetGeoVecCopy(cItem3->vInitTrans(), item->vInitTrans);
			if (1 == item->nKind)
			{
				cPBModel2->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
			}
			cItem3->nKind() = item->nKind;
			cItem3->nDFlag() = item->nDFlag;
			cItem3->pPBModel() = cPBModel2;
			cItem3->nRevivalCnt() = 0u;
			if (cItem3->nKind() == 17 && CustomKingdomUtil::IF(cItem3->nDFlag() & 1))
			{
				m_bEatPizza = true;
			}
			uint kind = item->nKind;
			if (kind - 2 <= 2)
			{
				cItem3->nRevivalCnt() = 120u;
			}
			PushBackItem(cItem3);
			if (cItem3->nKind() == 0 || 1 == cItem3->nKind())
			{
				m_nCookieMax++;
			}
			if (CustomKingdomUtil::IF(cItem3->nDFlag() & 1))
			{
				Invisible(cPBModel2);
			}
			if (cItem3->nKind() == 17 && m_bEatPizza)
			{
				cItem3->nDFlag() = 1u;
				Invisible(cPBModel2);
			}
			cPBModel2->UpdateMatrix();
			UpdateTrans(cPBModel2);
			cPBModel2->UpdateShadowModelMtx();
			num++;
		}
	}
	InitBossCookie();
}

// DOL 0x80182ba8 [audited: described by the 43rd-session pass]: blr (the models go with the cTaskModelEx part of the dtor chain, ReleaseTerm)
void cTaskItem::Term()
{
}

// DOL 0x80182bac [audited: described by the 43rd-session pass] (HitCheck and Homing are inline there; ends with UpdateFrame 0x80187098 unless paused / HOME, then
// cTaskModelEx::Execute 0x8018710c)
bool cTaskItem::Execute(cJetTask* pParam)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	uint num = 19u;
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (KingdomGlobal::PacnRollSys->IsTopMenuMode() || (!Constants::IS_PAC_PAUSE() && cAppG03PacnRollGame2->IsGame() && !cTaskCharPacman2->IsMiss()))
	{
		for (cItem* item : m_vecItem)
		{
			if (item->IsDFlag(2u))
			{
				if (!item->pBox()->IsBreak())
				{
					continue;
				}
				item->SetDFlag(0u);
			}
			else if (item->pGhost() != nullptr)
			{
				if (!item->pGhost()->IsDead())
				{
					item->SetDFlag(8u);
				}
				if (!cTaskCharPacman2->IsMiss() && item->IsDFlag(8u) && !item->IsDFlag(1u))
				{
					if (!item->IsDeadOld() && item->pGhost()->IsDead())
					{
						item->SetDFlag(1u);
						Invisible(item);
						item->nCnt() = 0u;
						item->pPBModel()->UpdateMatrix();
						UpdateTrans(item->pPBModel());
						item->pPBModel()->UpdateShadowModelMtx();
						EatItem(item);
						switch (item->nKind())
						{
						case 4u:
							cTaskCharPacman2->ChangePacn(0u);
							break;
						case 3u:
							cTaskCharPacman2->ChangePacn(1u);
							break;
						case 2u:
							cTaskCharPacman2->ChangePacn(2u);
							break;
						}
					}
					item->IsDeadOld() = item->pGhost()->IsDead();
					continue;
				}
				item->IsDeadOld() = item->pGhost()->IsDead();
			}
			if (item->IsDFlag(4u))
			{
				Homing(item);
				item->pPBModel()->UpdateMatrix();
				UpdateTrans(item->pPBModel());
				item->pPBModel()->UpdateShadowModelMtx();
			}
			uint num2 = HitCheck(item);
			if (!item->IsDFlag(1u) && !item->IsDFlag(16u))
			{
				if (1 == num2)
				{
					item->SetDFlag(1u);
					Invisible(item);
					item->nCnt() = 0u;
					EatItem(item);
					uint num3 = item->nKind();
					if (num3 - 2 <= 2)
					{
						num = item->nKind();
					}
				}
				if (2 == num2)
				{
					item->SetDFlag(4u);
				}
			}
			else if (!item->IsDFlag(16u) && 0 < item->nRevivalCnt() && ++item->nCnt() >= item->nRevivalCnt())
			{
				item->SetDFlag(0u);
				Visible(item);
				item->nCnt() = 0u;
				::Constants::jetGeoVecCopy(item->pPBModel()->vTrans(), item->vInitTrans());
				item->pPBModel()->UpdateMatrix();
				UpdateTrans(item->pPBModel());
				item->pPBModel()->UpdateShadowModelMtx();
				if (item->nKind() == 1 && Constants::PACN_APP()->nModeState() == 2)
				{
					SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE09_NEWSTAGE);
				}
			}
		}
		if (19 != num)
		{
			uint kind = 0u;
			switch (num)
			{
			case 4u:
				kind = 0u;
				break;
			case 3u:
				kind = 1u;
				break;
			case 2u:
				kind = 2u;
				break;
			default:
				CustomKingdomUtil::Assert("error");
				break;
			}
			cTaskCharPacman2->ChangePacn(kind);
		}
	}
	if (!Constants::IS_PAC_PAUSE())
	{
		UpdateFrame();
	}
	return cTaskModelEx::Execute(pParam);
}

// DOL 0x8018337c [audited: described by the 43rd-session pass]: b cTaskModelEx::Draw
void cTaskItem::Draw(int nStatus, cJetTask* pParam)
{
	cTaskModelEx::Draw(nStatus, pParam);
}

// DOL 0x80183380 [audited: class row done in the 43rd session; function not individually re-read]
string cTaskItem::Kind2Key(uint kind)
{
	switch (kind)
	{
	case 0u:
		return "BIN_ITEM_COOKIE_MDL";
	case 1u:
		return "BIN_ITEM_COOKIE_POW_MDL";
	case 2u:
		return "BIN_ITEM_CHOCO_KNIGHT_MDL";
	case 3u:
		return "BIN_ITEM_CHOCO_WING_MDL";
	case 4u:
		return "BIN_ITEM_CHOCO_PACN_MDL";
	case 5u:
		return "BIN_ITEM_APPLE_L_MDL";
	case 6u:
		return "BIN_ITEM_APPLE_S_MDL";
	case 7u:
		return "BIN_ITEM_GRAPE_L_MDL";
	case 8u:
		return "BIN_ITEM_GRAPE_S_MDL";
	case 9u:
		return "BIN_ITEM_BANANA_L_MDL";
	case 10u:
		return "BIN_ITEM_BANANA_S_MDL";
	case 11u:
		return "BIN_ITEM_HAMBURGER_L_MDL";
	case 12u:
		return "BIN_ITEM_HAMBURGER_S_MDL";
	case 13u:
		return "BIN_ITEM_PARFAIT_L_MDL";
	case 14u:
		return "BIN_ITEM_PARFAIT_S_MDL";
	case 15u:
		return "BIN_ITEM_MEAT_L_MDL";
	case 16u:
		return "BIN_ITEM_MEAT_S_MDL";
	case 17u:
		return "BIN_ITEM_PIZZA_MDL";
	case 18u:
		return "BIN_ITEM_SFLAG_MDL";
	default:
		CustomKingdomUtil::Assert("error");
		return string();
	}
}

// inline in Execute 0x80182bac: (pac +0x1f0 - item)^2 against (0.4 + pac radius +0x1dc)^2 [+ vacuum range +0x4f4 = 2]
uint cTaskItem::HitCheck(cItem* item)
{
	cTaskCharPacman* cTaskCharPacman2 = Constants::PACN_APP()->pTaskCharPacman();
	Vector4 v;
	::Constants::jetGeoVecSub(v, cTaskCharPacman2->vTrans(), item->pPBModel()->vTrans());
	float num = ::Constants::jetGeoVecInnerProduct(v, v);
	float num2 = cTaskCharPacman2->fRadius() + 0.4f;
	num2 *= num2;
	uint result = 0u;
	if (!cTaskCharPacman2->IsMiss() && num <= num2 + cTaskCharPacman2->GetParamCommon()->fVacuumRange)
	{
		result = 2u;
		if (num <= num2)
		{
			result = 1u;
		}
	}
	return result;
}

// inline in Execute 0x80182bac: speed += 0.02 (FLOAT_8047c7fc), moves only when the squared distance >= 0x01000000
void cTaskItem::Homing(cItem* item)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	Vector4 v;
	::Constants::jetGeoVecSub(v, cAppG03PacnRollGame2->pTaskCharPacman()->vTrans(), item->pPBModel()->vTrans());
	item->fSpeed() += 0.02f;
	if (Constants::IsVecNormalize(v))
	{
		if (::Constants::jetGeoVecInnerProduct(v, v) > item->fSpeed() * item->fSpeed())
		{
			::Constants::jetGeoVecNormalize(v, v);
			::Constants::jetGeoVecScale(v, v, item->fSpeed());
		}
		::Constants::jetGeoVecAdd(item->pPBModel()->vTrans(), item->pPBModel()->vTrans(), v);
	}
}

// DOL 0x8018348c [audited: class row done in the 43rd session; function not individually re-read]
void cTaskItem::EatItem(cItem* item)
{
	cAppG03PacnRollGame* cAppG03PacnRollGame2 = Constants::PACN_APP();
	cTaskCharPacman* cTaskCharPacman2 = cAppG03PacnRollGame2->pTaskCharPacman();
	if (item->nKind() == 0 || 1 == item->nKind())
	{
		m_nAteCookie++;
		m_nHaveCookie++;
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE015_COOKIE);
	}
	if (1 == item->nKind())
	{
		Constants::PACN_APP()->IjikeRegist();
	}
	bool flag = false;
	switch (item->nKind())
	{
	case 5u:
	case 7u:
	case 9u:
	case 11u:
	case 13u:
	case 15u:
		cTaskCharPacman2->RecoverHp();
		flag = true;
		break;
	case 6u:
	case 8u:
	case 10u:
	case 12u:
	case 14u:
	case 16u:
		cTaskCharPacman2->RecoverHp(1u);
		flag = true;
		break;
	case 17u:
		cTaskCharPacman2->GainHpLimit(1u);
		cTaskCharPacman2->RecoverHp();
		m_bEatPizza = true;
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE050_PIZA);
		break;
	case 18u:
		cTaskCharPacman2->GainLife(1u);
		SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE002_FLAG);
		break;
	}
	if (flag)
	{
		if (cAppG03PacnRollGame2->pTaskCharPacman()->IsState(1u))
		{
			// DOL: Goal_3 into the handle +0x1dc when it no longer plays (SoundSeBank::IsPlaying 0x80026424), else Goal_2 (43rd: was
			// "handle == null", so every later fruit while dashing played Goal_2)
			if (!SoundManager::get_SE()->IsPlaying(m_SndYahoo))
			{
				m_SndYahoo = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE109_GOAL_3);
			}
			else
			{
				m_SndYahoo = SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE108_GOAL_2);
			}
		}
		else
		{
			SoundManager::get_SE()->PlaySE(ESEKind::PACN_ROLL_SND_SE016_FLUIT);
		}
	}
	// (43rd: the C#'s AddGetItemCnt here fed a Steam statistic; the DOL has none)
}

void cTaskItem::PushBackItem(cItem* item)
{
	CustomKingdomUtil::Assert(item != nullptr, "nullpo");
	m_vecItem.push_back(item);
}

// DOL 0x80183684 [audited: described by the 43rd-session pass] (angles through CosFIdx / SinFIdx: (deg * pi/180 - rotY) * 256/2pi)
bool cTaskItem::SetBoxItem(Vector4 trans, float rotY, uint treasure, cTaskBgBox* box)
{
	CustomKingdomUtil::Assert("nullpo");
	CustomKingdomUtil::Assert(box != nullptr, "nullpo");
	float num = 0.5f;
	uint num2;
	switch (treasure)
	{
	case 1u:
		num2 = 0u;
		break;
	case 2u:
		num2 = 0u;
		break;
	case 3u:
		num2 = 0u;
		break;
	case 4u:
		num2 = 0u;
		break;
	case 5u:
		num2 = 1u;
		break;
	case 6u:
		num2 = 2u;
		break;
	case 7u:
		num2 = 3u;
		break;
	case 8u:
		num2 = 4u;
		break;
	case 9u:
		num2 = 5u;
		break;
	case 10u:
		num2 = 6u;
		break;
	case 11u:
		num2 = 7u;
		break;
	case 12u:
		num2 = 8u;
		break;
	case 13u:
		num2 = 9u;
		break;
	case 14u:
		num2 = 10u;
		break;
	case 15u:
		num2 = 11u;
		break;
	case 16u:
		num2 = 12u;
		break;
	case 17u:
		num2 = 13u;
		break;
	case 18u:
		num2 = 14u;
		break;
	case 19u:
		num2 = 15u;
		break;
	case 20u:
		num2 = 16u;
		break;
	case 21u:
		num2 = 17u;
		break;
	case 22u:
		num2 = 18u;
		break;
	default:
		return false;
	}
	uint num3 = ((treasure == 2u) ? 3u : ((treasure == 3u) ? 6u : ((treasure == 4u) ? 9u : (1u))));
	int idx = 0;
	for (uint num4 = 0u; num4 < num3; num4++)
	{
		bool isUpdateFrame = num2 == 1;
		CreateModelEx(idx, isUpdateFrame);
		cTaskModelEx::cPBModel* cPBModel = pPBModel(idx);
		cPBModel->RegistModel(Kind2Key(num2));
		cPBModel->IsDrawShadowDepth(true);
		cPBModel->SetLightSetIdx(1);
		cPBModel->SetFogIdx(0);
		cPBModel->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
		Visible(cPBModel);
		::Constants::jetGeoVecCopy(cPBModel->vTrans(), trans);
		if (1 == num2)
		{
			cPBModel->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
		}
		switch (num3)
		{
		case 3u:
			cPBModel->vTrans()[0] += Mathf::Cos(::Constants::JET_DEG2RAD(120 * num4 + 30) - rotY) * 0.5f;
			cPBModel->vTrans()[2] += Mathf::Sin(::Constants::JET_DEG2RAD(120 * num4 + 30) - rotY) * 0.5f;
			cPBModel->vTrans()[1] += num;
			break;
		case 6u:
			cPBModel->vTrans()[0] += Mathf::Cos(::Constants::JET_DEG2RAD(60 * num4) - rotY) * 1.0f;
			cPBModel->vTrans()[2] += Mathf::Sin(::Constants::JET_DEG2RAD(60 * num4) - rotY) * 1.0f;
			cPBModel->vTrans()[1] += num;
			break;
		case 9u:
			cPBModel->vTrans()[0] += Mathf::Cos(::Constants::JET_DEG2RAD(40 * num4 + 190) - rotY) * 1.3f;
			cPBModel->vTrans()[2] += Mathf::Sin(::Constants::JET_DEG2RAD(40 * num4 + 190) - rotY) * 1.3f;
			cPBModel->vTrans()[1] += num;
			break;
		default:
			cPBModel->vTrans()[1] += num;
			break;
		}
		cItem* cItem2 = new cItem();
		::Constants::jetGeoVecCopy(cItem2->vInitTrans(), cPBModel->vTrans());
		cItem2->nKind() = num2;
		cItem2->nDFlag() = 2u;
		cItem2->pPBModel() = cPBModel;
		cItem2->pBox() = box;
		if (num2 - 2 <= 2)
		{
			cItem2->nRevivalCnt() = 120u;
		}
		if (cItem2->nKind() == 17 && m_bEatPizza)
		{
			cItem2->nDFlag() = 1u;
			Invisible(cPBModel);
		}
		if (num2 == 0 || 1 == num2)
		{
			m_nCookieMax++;
		}
		cPBModel->UpdateMatrix();
		UpdateTrans(cPBModel);
		cPBModel->UpdateShadowModelMtx();
		PushBackItem(cItem2);
	}
	return true;
}

// DOL 0x801840f0 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskItem::SetEnemyItem(uint kind, cTaskCharGhost* ghost, Vector4 trans)
{
	CustomKingdomUtil::Assert(ghost != nullptr, "nullpo");
	if (!ghost->IsDead() || KingdomGlobal::PacnRollSys->IsBossStage())
	{
		int idx = 0;
		CreateModelEx(idx);
		cTaskModelEx::cPBModel* cPBModel = pPBModel(idx);
		cPBModel->RegistModel(Kind2Key(kind));
		cPBModel->IsDrawShadowDepth(true);
		cPBModel->SetLightSetIdx(1);
		cPBModel->SetFogIdx(0);
		cPBModel->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
		Invisible(cPBModel);
		cItem* cItem2 = new cItem();
		::Constants::jetGeoVecCopy(cPBModel->vTrans(), trans);
		::Constants::jetGeoVecCopy(cItem2->vInitTrans(), trans);
		if (1 == kind)
		{
			cPBModel->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
		}
		cItem2->nKind() = kind;
		cItem2->pPBModel() = cPBModel;
		cItem2->pGhost() = ghost;
		cItem2->IsDeadOld() = ghost->IsDead();
		cItem2->nDFlag() = 8u;
		if (kind - 2 <= 2)
		{
			cItem2->nRevivalCnt() = 120u;
		}
		if (cItem2->nKind() == 17 && m_bEatPizza)
		{
			cItem2->nDFlag() = 1u;
			Invisible(cPBModel);
		}
		if (kind == 0 || 1 == kind)
		{
			m_nCookieMax++;
		}
		cPBModel->UpdateMatrix();
		UpdateTrans(cPBModel);
		cPBModel->UpdateShadowModelMtx();
		PushBackItem(cItem2);
	}
}

// DOL 0x80184898 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskItem::SetRetryData()
{
	KingdomGlobal::PacnRollSys->pRetryData()->nAteCookie = m_nAteCookie;
	KingdomGlobal::PacnRollSys->pRetryData()->nHaveCookie = m_nHaveCookie;
	for (cItem* item : m_vecItem)
	{
		if ((item->pBox() != nullptr && !item->pBox()->IsBreak()) || (item->pGhost() != nullptr && !item->pGhost()->IsDead()))
		{
			continue;
		}
		cG03PacnRollSys::sRetryItem* sRetryItem = new cG03PacnRollSys::sRetryItem();
		sRetryItem->nKind = item->nKind();
		sRetryItem->nDFlag = item->nDFlag();
		::Constants::jetGeoVecCopy(sRetryItem->vInitTrans, item->vInitTrans());
		uint num = item->nKind();
		if (num <= 1 || num - 17 <= 1)
		{
			if (!CustomKingdomUtil::IF(sRetryItem->nDFlag & 1))
			{
				sRetryItem->nDFlag = 0u;
			}
		}
		else
		{
			sRetryItem->nDFlag = 0u;
		}
		KingdomGlobal::PacnRollSys->pRetryData()->vecItem.push_back(sRetryItem);
	}
}

// DOL 0x80184ddc [audited: described by the 43rd-session pass] (BIN_CSV_BOSS_PARAM = CRC 0xbb63a2a0, row "COOKIE_REVIVAL" column 1)
int cTaskItem::BossCookieRevivalTime()
{
	cKingdomCSV* cKingdomCSV2 = new cKingdomCSV();
	if (CustomKingdomUtil::IF("BIN_CSV_BOSS_PARAM") && !KingdomGlobal::KingSys->SearchData("BIN_CSV_BOSS_PARAM").empty())
	{
		cKingdomCSV2->RegistData("BIN_CSV_BOSS_PARAM");
		int _out = 0;
		for (int i = 1; i < cKingdomCSV2->nRow(); i++)
		{
			string _out2;
			if (cKingdomCSV2->Str(i, 0, _out2, 128u) && CustomKingdomUtil::strncmp(_out2, "COOKIE_REVIVAL", 14) == 0)
			{
				cKingdomCSV2->S32(i, 1, _out);
				break;
			}
		}
		return _out;
	}
	return 0;
}

// DOL 0x80184f40 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskItem::InitBossCookie()
{
	for (int i = 0; i < (int)std::size(m_pvecBossItem); i++)
	{
		m_pvecBossItem[i].clear();
	}
	uint num = (uint)BossCookieRevivalTime();
	if (num == 0)
	{
		num = 600u;
	}
	for (uint num2 = 0u; num2 < 3; num2++)
	{
		sKingdomData* sKingdomData2 = KingdomGlobal::KingSys->SearchDataEx(s_aBossCookie[num2]);
		if (sKingdomData2 == nullptr || 0 >= sKingdomData2->nSize)
		{
			break;
		}
		std::vector<sSaveDataItem*> outParams;
		int outParamNum;
		int _discard8;
		ReadFileUtil::Convert<sSaveDataItem>(sKingdomData2->pBin, outParams, outParamNum, _discard8);
		int count = (int)m_vecItem.size();
		for (int j = 0; j < outParamNum; j++)
		{
			cItem* cItem2 = new cItem();
			bool isUpdateFrame = outParams[j]->kind == 1;
			CreateModel(j + count, isUpdateFrame);
			cTaskModelEx::cPBModel* cPBModel = pPBModel(j + count);
			cPBModel->RegistModel(Kind2Key(outParams[j]->kind));
			cPBModel->IsDrawShadowDepth(true);
			cPBModel->SetLightSetIdx(1);
			cPBModel->SetFogIdx(0);
			cPBModel->RegistShadowModel("BIN_ITEMS_SHADOW_MDL");
			Invisible(cPBModel);
			::Constants::jetGeoVecCopy(cPBModel->vTrans(), outParams[j]->trans);
			::Constants::jetGeoVecCopy(cItem2->vInitTrans(), outParams[j]->trans);
			if (1 == outParams[j]->kind)
			{
				cPBModel->RegistAnm(0u, "BIN_ITEM_COOKIE_POW_TPA");
			}
			cItem2->nKind() = outParams[j]->kind;
			cItem2->nDFlag() = 16u;
			cItem2->pPBModel() = cPBModel;
			cItem2->nRevivalCnt() = num;
			cPBModel->UpdateMatrix();
			UpdateTrans(cPBModel);
			cPBModel->UpdateShadowModelMtx();
			PushBackItem(cItem2);
			m_pvecBossItem[num2].push_back((uint)((int)m_vecItem.size() - 1));
		}
	}
}

// DOL 0x80185a38 [audited: class row done in the 43rd session; function not individually re-read]
void cTaskItem::SetBossCookie(uint nLife)
{
	for (uint num = 0u; num < 3; num++)
	{
		for (uint item : m_pvecBossItem[num])
		{
			cItem* cItem2 = m_vecItem[(int)item];
			if (nLife == 3 - num)
			{
				cItem2->SetDFlag(0u);
				Visible(cItem2);
				cItem2->nCnt() = 0u;
				::Constants::jetGeoVecCopy(cItem2->pPBModel()->vTrans(), cItem2->vInitTrans());
			}
			else
			{
				cItem2->SetDFlag(16u);
				Invisible(cItem2);
			}
		}
	}
}

void cTaskItem::UpdateTrans(cTaskModelEx::cPBModel* model)
{
	// TODO(unity): model.pScnMdlTransform().position = model.vTrans().GetUnityPos();
}

void cTaskItem::Visible(cItem* item)
{
	Visible(item->pPBModel());
}

void cTaskItem::Visible(cTaskModelEx::cPBModel* model)
{
	model->Visible();
	if (model->get_ShadowModel() != nullptr)
	{
		model->get_ShadowModel()->Visible();
	}
}

void cTaskItem::Invisible(cItem* item)
{
	Invisible(item->pPBModel());
}

void cTaskItem::Invisible(cTaskModelEx::cPBModel* model)
{
	model->Invisible();
	if (model->get_ShadowModel() != nullptr)
	{
		model->get_ShadowModel()->Invisible();
	}
}

}
