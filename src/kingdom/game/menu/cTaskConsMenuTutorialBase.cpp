#include "kingdom/game/menu/cTaskConsMenuTutorialBase.hpp"
#include <cstdlib>
#include <cstdio>
#include "gfx/FontWriter.hpp"
#include "gfx/Gfx.hpp"
#include "GamePad.hpp"
#include <ESEKind.hpp>
#include <SoundManager.hpp>

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomCSV.hpp"
#include "kingdom/game/menu/sTutorialData.hpp"
#include "kingdom/game/menu/sTutorialPage.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "MotosFrontUIManager.hpp"
#include "MotosUIRuleButtonSet.hpp"
#include "MotosUIRuleImage.hpp"
#include "MotosUIRuleText.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>

#include <SoundManager.hpp>

namespace kingdom::game::menu
{

uint cTaskConsMenuTutorialBase::_nGetOrgAntNo(uint nNo)
{
	if (nNo == std::numeric_limits<uint>::max())
	{
		CustomKingdomUtil::Assert(0 <= m_nNowPageNo && m_nNowPageNo < m_nMaxPageNo, "PageOver");
		return 5 + m_nNowPageNo;
	}
	CustomKingdomUtil::Assert(0 <= nNo && nNo < m_nMaxPageNo, "PageOver");
	return 5 + nNo;
}

sTutorialPage* cTaskConsMenuTutorialBase::pGetPageData(uint nPageNo)
{
	CustomKingdomUtil::Assert(0 <= nPageNo && nPageNo < nMaxPageNo(), "Page Error");
	return pData()->pPage[nPageNo];
}

uint& cTaskConsMenuTutorialBase::nMaxPageNo()
{
	return m_nMaxPageNo;
}

uint& cTaskConsMenuTutorialBase::nNowPageNo()
{
	return m_nNowPageNo;
}

sTutorialData*& cTaskConsMenuTutorialBase::pData()
{
	return m_pTutorialData;
}

Vector4& cTaskConsMenuTutorialBase::vFontPos(int n)
{
	return m_vFontPos[n];
}

int& cTaskConsMenuTutorialBase::nCsvRow()
{
	return m_nCsvRow;
}

cKingdomCSV*& cTaskConsMenuTutorialBase::CsvText()
{
	return m_CsvText;
}

cTaskConsMenuTutorialBase::~cTaskConsMenuTutorialBase()
{
}

bool cTaskConsMenuTutorialBase::IsDeleteEnd()
{
	return m_nState == 6;
}

cTaskConsMenuTutorialBase::cTaskConsMenuTutorialBase()
	: NUMBER_ANM_LIST{{0, -1, -1, -1, 1}, {2, -1, -1, -1, 3}, {4, -1, -1, -1, 5}, {6, -1, -1, -1, 7}, {8, -1, -1, -1, 9}, {10, -1, -1, -1, 11}, {12, -1, -1, -1, 13}, {14, -1, -1, -1, 15}, {16, -1, -1, -1, 17}}
{
	m_nState = 6;
	m_nStep = 0;
	nNowPageNo() = 0u;
	nCsvRow() = 0;
	m_CsvText = new cKingdomCSV();
}

cTaskConsMenuTutorialBase::cTaskConsMenuTutorialBase(string CommonKey, sTutorialData* pTutorialData)
	: NUMBER_ANM_LIST{{0, -1, -1, -1, 1}, {2, -1, -1, -1, 3}, {4, -1, -1, -1, 5}, {6, -1, -1, -1, 7}, {8, -1, -1, -1, 9}, {10, -1, -1, -1, 11}, {12, -1, -1, -1, 13}, {14, -1, -1, -1, 15}, {16, -1, -1, -1, 17}}
{
	_cTaskConsMenuTutorialBase(CommonKey, pTutorialData);
}

void cTaskConsMenuTutorialBase::_cTaskConsMenuTutorialBase(string CommonKey, sTutorialData* pTutorialData)
{
	UnityUIButton = MotosFrontUIManager::Get<MotosUIRuleButtonSet>(MotosFrontUIManager::EUIIndex::RuleButton);
	m_nState = 6;
	m_nStep = 0;
	nNowPageNo() = 0u;
	nCsvRow() = 0;
	RegistData(CommonKey, pTutorialData);
}

void cTaskConsMenuTutorialBase::RegistData(string CommonKey, sTutorialData* pTutorialData)
{
	pData() = pTutorialData;
	m_Text.Load(pData()->TextDataName.c_str());
	nMaxPageNo() = pData()->nPageMax;
	CustomKingdomUtil::Assert(pData()->nPageMax < 10, "OriginalData Over");
	int drawLayer = 0; // TODO(unity): drawLayer = LayerMask.NameToLayer("FrontUI")
	for (uint num = 0u; num < 19; num++)
	{
		if (5 > num || num >= 15)
		{
			pAnita(num)->RegistData(ResourceDatabase::GetPath(CommonKey), drawLayer);
			pAnita(num)->SetState(1u, false);
			pAnita(num)->SetAnimNo(0);
		}
	}
	pAnita(2u)->GetLayerPos(pAnita(2u)->vTrans(), PAGE_SET_LAY[0], 0);
	pAnita(3u)->GetLayerPos(pAnita(3u)->vTrans(), PAGE_SET_LAY[1], 0);
	pAnita(4u)->GetLayerPos(pAnita(4u)->vTrans(), 1u, 0);
	// DOL 0x80145f64 / 0x80145f80: where the page text goes (anita 0, layer 0, pattern 1 for the first page, pattern 2 for the others)
	pAnita(0u)->GetLayerPos(m_vFontPos[0], 0u, 1);
	pAnita(0u)->GetLayerPos(m_vFontPos[1], 0u, 2);
	for (uint num2 = 0u; num2 < nMaxPageNo(); num2++)
	{
		pAnita(_nGetOrgAntNo(num2))->RegistData(ResourceDatabase::GetPath(pGetPageData(num2)->Key), drawLayer);
		pAnita(_nGetOrgAntNo(num2))->SetState(1u, false);
		pAnita(_nGetOrgAntNo(num2))->SetAnimNo(0);
	}
	pAnita(0u)->GetLayerPos(vFontPos(0), 0u, 1);
	pAnita(0u)->GetLayerPos(vFontPos(1), 0u, 2);
}

void cTaskConsMenuTutorialBase::Start()
{
	nNowPageNo() = 0u;
	nMaxPageNo() = pData()->nPageMax;
	m_nState = 0;
	m_nStep = 0;
	nCsvRow() = pGetPageData(nNowPageNo())->nTextRow;
}

void cTaskConsMenuTutorialBase::AllWaitState()
{
	pAnita(16u)->SetAnimNo(CLOSE_ANM_LIST[1]);
	pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[1]);
	pAnita(17u)->SetAnimNo(PREV_ANM_LIST[1]);
}

// DOL 0x80146058 (virtual slot 19): the pointer (cursor slot 26 = GetPointer; its "valid" result is ignored, an invalid pointer reads as (0, 0)) is tested
// against the hit pattern of the close (anita 16, pattern 3), previous (anita 17, pattern 5) and next button (anita 18, pattern 4), in that order;
// the last hit wins the highlight. A press (trig & 0x860) on a hit button leaves the page: close -> state 4, previous -> 3, next -> 2.
// The buttons then play their normal (table index 1) or highlighted (index 2) animation without resetting; a new highlight plays the select cue.
bool cTaskConsMenuTutorialBase::DpdSelect()
{
	if (m_pCursor == nullptr) return false;
	float px = 0.0f, py = 0.0f;
	m_pCursor->GetPointer(px, py);
	const GamePad::State& pad = GamePad::Get(m_pCursor->GetPadNo());   // get_pad1(cursor vtable +0x5c = GetPadNo)
	const Vector4 p((float)(int)(px * (float)gfx::SCREEN_W * 0.5f), (float)(int)(-py * (float)gfx::SCREEN_H * 0.5f), 0.0f, 1.0f);
	int sel = 0;
	if (pAnita(16u)->IsHit(p, 3))
	{
		sel = 3;
		if (pad.trig & 0x860) m_nState = 4;
	}
	if (pAnita(17u)->IsHit(p, 5))
	{
		sel = 2;
		if (pad.trig & 0x860) m_nState = 3;
	}
	if (pAnita(18u)->IsHit(p, 4))
	{
		sel = 1;
		if (pad.trig & 0x860) m_nState = 2;
	}
	pAnita(16u)->SetAnimNo(CLOSE_ANM_LIST[sel == 3 ? 2 : 1], false, 0.0f);
	pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[sel == 1 ? 2 : 1], false, 0.0f);
	pAnita(17u)->SetAnimNo(PREV_ANM_LIST[sel == 2 ? 2 : 1], false, 0.0f);
	if (sel != m_nPointSel && sel != 0)
	{
		SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE01_SELECT); // UNVERIFIED cue (the DOL passes the string at r2 - 0x5980)
	}
	m_nPointSel = sel;
	return sel != 0;
}

void cTaskConsMenuTutorialBase::KeySelect()
{
	int padNumFirst = InGamePadManager::GetPadNumFirst();
	if (InGamePadManager::GetCancelKeyDown(padNumFirst))
	{
		m_nState = 4;
	}
	else if (InGamePadManager::GetKeyDownFlag(padNumFirst, (EPadKeyFlag)((int)EPadKeyFlag::eKEY_LEFT | (int)EPadKeyFlag::eKEY_LSTICK_LEFT)))
	{
		if (IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV))
		{
			m_nState = 3;
		}
	}
	else if (InGamePadManager::GetKeyDownFlag(padNumFirst, (EPadKeyFlag)((int)EPadKeyFlag::eKEY_RIGHT | (int)EPadKeyFlag::eKEY_LSTICK_RIGHT)) && IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
	{
		m_nState = 2;
	}
}

// DOL 0x801465a4 (vtable +0x58; the owner calls it every frame: g04 console 0x801b1434 mode 2)
void cTaskConsMenuTutorialBase::Select()
{
	if (m_nState == 1 && !DpdSelect())
	{
		KeySelect();
	}
}

bool cTaskConsMenuTutorialBase::Execute(cJetTask* pParam)
{
	switch (m_nState)
	{
	case 0:
		if (m_nStep == 0)
		{
			pAnita(0u)->SetAnimNo(BG_ANM_LIST[0]);
			pAnita(1u)->SetAnimNo(PROCESS_ANM_LIST[0]);
			pAnita(2u)->SetAnimNo(NUMBER_ANM_LIST[nNowPageNo()][0]);
			pAnita(3u)->SetAnimNo(NUMBER_ANM_LIST[nMaxPageNo() - 1][0]);
			pAnita(4u)->SetAnimNo(SLASH_ANM_LIST[0]);
			pAnita(16u)->SetAnimNo(CLOSE_ANM_LIST[0]);
			pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[0]);
			pAnita(17u)->SetAnimNo(PREV_ANM_LIST[0]);
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::Close, MotosUIRuleButtonSet::EAnim::In);
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::In);
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::In);
			}
			pAnita(0u)->SetState(1u);
			pAnita(1u)->SetState(1u, TextVisibleValue(true));
			pAnita(2u)->SetState(1u);
			pAnita(3u)->SetState(1u);
			pAnita(4u)->SetState(1u);
			pAnita(16u)->SetState(1u, ButtonVisibleValue(true));
			if (nNowPageNo() >= nMaxPageNo() - 1)
			{
				pAnita(17u)->SetState(1u, ButtonVisibleValue(true));
				if (UnityUIButton != nullptr)
				{
					UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::PrevPage, true);
				}
			}
			else
			{
				if (UnityUIButton != nullptr)
				{
					UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::PrevPage, false);
				}
			}
			if (nNowPageNo() == 0)
			{
				pAnita(18u)->SetState(1u, ButtonVisibleValue(true));
				if (UnityUIButton != nullptr)
				{
					UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::NextPage, true);
				}
			}
			else
			{
				if (UnityUIButton != nullptr)
				{
					UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::NextPage, false);
				}
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->SetEnable(true);
			}
			m_nStep++;
		}
		else
		{
			if (m_nStep != 1)
			{
				break;
			}
			bool flag2 = true;
			if (flag2)
			{
				flag2 = pAnita(0u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(2u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(3u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(4u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(16u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(18u)->IsState(65536u);
			}
			if (flag2)
			{
				flag2 = pAnita(17u)->IsState(65536u);
			}
			if (nullptr != UnityUIButton)
			{
				if (flag2)
				{
					flag2 = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::Close, MotosUIRuleButtonSet::EAnim::In);
				}
				if (flag2)
				{
					flag2 = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::In);
				}
				if (flag2)
				{
					flag2 = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::In);
				}
			}
			if (flag2)
			{
				if (UnityUIImage != nullptr)
				{
					UnityUIImage->ResetVisible();
				}
				if (UnityUIImage != nullptr)
				{
					UnityUIImage->SetEnable(true);
				}
				if (pGetPageData(nNowPageNo())->nAnmNo != -1)
				{
					pAnita(_nGetOrgAntNo())->SetAnimNo(pGetPageData(nNowPageNo())->nAnmNo);
					pAnita(_nGetOrgAntNo())->SetState(1u, ImageVisibleValue(true));
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->SetVisible(nNowPageNo(), true);
					}
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->PlayAnim(nNowPageNo(), MotosUIRuleImage::EAnim::In);
					}
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->ResetVisible();
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetEnable(true);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetVisible(nNowPageNo(), true);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->PlayAnim(nNowPageNo(), MotosUIRuleText::EAnim::In);
				}
				m_nState = 1;
				m_nStep = 0;
			}
		}
		break;
	case 2:
		if (m_nStep == 0)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[3]);
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::Push);
			}
			if (pGetPageData(nNowPageNo())->nAnmNo != -1)
			{
				pAnita(15u)->SetAnimNo(COVER_ANM_LIST[0]);
				pAnita(15u)->SetState(1u);
			}
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			if (IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
			{
				uint num2 = nNowPageNo();
				nNowPageNo()++;
				nCsvRow() = pGetPageData(nNowPageNo())->nTextRow;
				pAnita(0u)->SetAnimNo(42);
				if (pAnita(1u)->IsState(1u))
				{
					pAnita(1u)->SetAnimNo(PROCESS_ANM_LIST[4]);
				}
				if (nNowPageNo() != 0 && !IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV))
				{
					pAnita(17u)->SetAnimNo(PREV_ANM_LIST[0]);
					pAnita(17u)->SetState(1u, ButtonVisibleValue(true));
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::PrevPage, true);
					}
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::In);
					}
				}
				if (nNowPageNo() >= nMaxPageNo() - 1 && IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
				{
					pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[4]);
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::Out);
					}
				}
				pAnita(_nGetOrgAntNo(num2))->SetState(1u, ImageVisibleValue(false));
				if (UnityUIImage != nullptr)
				{
					UnityUIImage->SetVisible(num2, false);
				}
				if (pGetPageData(nNowPageNo())->nAnmNo != -1)
				{
					pAnita(_nGetOrgAntNo())->SetAnimNo(pGetPageData(nNowPageNo())->nAnmNo);
					pAnita(_nGetOrgAntNo())->SetState(1u, ImageVisibleValue(true));
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->SetVisible(nNowPageNo(), true);
					}
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->PlayAnim(nNowPageNo(), MotosUIRuleImage::EAnim::In);
					}
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetVisible(num2, false);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetVisible(nNowPageNo(), true);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->PlayAnim(nNowPageNo(), MotosUIRuleText::EAnim::In);
				}
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			bool flag3 = true;
			if (pAnita(18u)->GetAnimNo() == NEXT_ANM_LIST[4] && IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
			{
				if (IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
				{
					pAnita(18u)->SetState(1u, ButtonVisibleValue(false));
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::NextPage, false);
					}
				}
				flag3 = false;
			}
			if (pAnita(1u)->GetAnimNo() == PROCESS_ANM_LIST[4] && pAnita(1u)->IsState(1u))
			{
				if (pAnita(1u)->IsState(65536u))
				{
					pAnita(1u)->SetState(1u, TextVisibleValue(false));
				}
				flag3 = false;
			}
			if (pAnita(0u)->IsState(65536u) & flag3)
			{
				pAnita(2u)->SetAnimNo(NUMBER_ANM_LIST[nNowPageNo()][0]);
				if (pAnita(15u)->IsState(1u))
				{
					pAnita(15u)->SetAnimNo(COVER_ANM_LIST[4]);
					m_nStep++;
				}
				else
				{
					m_nState = 1;
					m_nStep = 0;
				}
			}
		}
		else if (m_nStep == 3 && pAnita(15u)->IsState(65536u))
		{
			pAnita(15u)->SetState(1u, false);
			m_nState = 1;
			m_nStep = 0;
		}
		break;
	case 3:
		if (m_nStep == 0)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE02_DECIDE);
			pAnita(17u)->SetAnimNo(PREV_ANM_LIST[3]);
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::Push);
			}
			if (pGetPageData(nNowPageNo())->nAnmNo != -1)
			{
				pAnita(15u)->SetAnimNo(COVER_ANM_LIST[0]);
				pAnita(15u)->SetState(1u);
			}
			m_nStep++;
		}
		else if (m_nStep == 1)
		{
			if (IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV))
			{
				uint num3 = nNowPageNo();
				nNowPageNo()--;
				nCsvRow() = pGetPageData(nNowPageNo())->nTextRow;
				pAnita(0u)->SetAnimNo(42);
				if (nNowPageNo() == 0)
				{
					pAnita(1u)->SetAnimNo(PROCESS_ANM_LIST[0]);
					pAnita(1u)->SetState(1u, TextVisibleValue(true));
				}
				if (nNowPageNo() < nMaxPageNo() && !IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT))
				{
					pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[0]);
					pAnita(18u)->SetState(1u, ButtonVisibleValue(true));
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::NextPage, true);
					}
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::In);
					}
				}
				if (nNowPageNo() == 0 && IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV))
				{
					pAnita(17u)->SetAnimNo(PREV_ANM_LIST[4]);
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::Out);
					}
				}
				pAnita(_nGetOrgAntNo(num3))->SetState(1u, ImageVisibleValue(false));
				if (UnityUIImage != nullptr)
				{
					UnityUIImage->SetVisible(num3, false);
				}
				if (pGetPageData(nNowPageNo())->nAnmNo != -1)
				{
					pAnita(_nGetOrgAntNo())->SetAnimNo(pGetPageData(nNowPageNo())->nAnmNo);
					pAnita(_nGetOrgAntNo())->SetState(1u, ImageVisibleValue(true));
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->SetVisible(nNowPageNo(), true);
					}
					if (UnityUIImage != nullptr)
					{
						UnityUIImage->PlayAnim(nNowPageNo(), MotosUIRuleImage::EAnim::In);
					}
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetVisible(num3, false);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetVisible(nNowPageNo(), true);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->PlayAnim(nNowPageNo(), MotosUIRuleText::EAnim::In);
				}
				m_nStep++;
			}
		}
		else if (m_nStep == 2)
		{
			bool flag4 = true;
			if (pAnita(17u)->GetAnimNo() == PREV_ANM_LIST[4] && IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV))
			{
				if (pAnita(17u)->IsState(65536u))
				{
					pAnita(17u)->SetState(1u, ButtonVisibleValue(false));
					if (UnityUIButton != nullptr)
					{
						UnityUIButton->SetVisible(MotosUIRuleButtonSet::EButtonType::PrevPage, false);
					}
				}
				flag4 = false;
			}
			if (pAnita(0u)->IsState(65536u) & flag4)
			{
				pAnita(2u)->SetAnimNo(NUMBER_ANM_LIST[nNowPageNo()][0]);
				if (pAnita(15u)->IsState(1u))
				{
					pAnita(15u)->SetAnimNo(COVER_ANM_LIST[4]);
					m_nStep++;
				}
				else
				{
					m_nState = 1;
					m_nStep = 0;
				}
			}
		}
		else if (m_nStep == 3 && pAnita(15u)->IsState(65536u))
		{
			pAnita(15u)->SetState(1u, false);
			m_nState = 1;
			m_nStep = 0;
		}
		break;
	case 4:
		if (m_nStep == 0)
		{
			SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE03_CANCEL);
			pAnita(16u)->SetAnimNo(CLOSE_ANM_LIST[3]);
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::Close, MotosUIRuleButtonSet::EAnim::Push);
			}
			if (IsVisibleImage(nNowPageNo()))
			{
				pAnita(15u)->SetAnimNo(COVER_ANM_LIST[0]);
				pAnita(15u)->SetState(1u);
			}
			if (pAnita(1u)->IsState(1u))
			{
				pAnita(1u)->SetAnimNo(PROCESS_ANM_LIST[4]);
			}
			m_nStep++;
		}
		else if (m_nStep == 1 && IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID::eANT_CLOSE))
		{
			pAnita(_nGetOrgAntNo())->SetState(1u, ImageVisibleValue(false));
			if (UnityUIImage != nullptr)
			{
				UnityUIImage->SetVisible(nNowPageNo(), false);
			}
			pAnita(1u)->SetState(1u, TextVisibleValue(false));
			m_nState = 5;
			m_nStep = 0;
		}
		break;
	case 5:
		if (m_nStep == 0)
		{
			pAnita(0u)->SetAnimNo(BG_ANM_LIST[4]);
			pAnita(2u)->SetAnimNo(NUMBER_ANM_LIST[nNowPageNo()][4]);
			pAnita(3u)->SetAnimNo(NUMBER_ANM_LIST[nMaxPageNo() - 1][4]);
			pAnita(4u)->SetAnimNo(SLASH_ANM_LIST[4]);
			pAnita(16u)->SetAnimNo(CLOSE_ANM_LIST[4]);
			pAnita(18u)->SetAnimNo(NEXT_ANM_LIST[4]);
			pAnita(17u)->SetAnimNo(PREV_ANM_LIST[4]);
			if (pAnita(15u)->IsState(1u))
			{
				pAnita(15u)->SetAnimNo(41);
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::Close, MotosUIRuleButtonSet::EAnim::Out);
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::Out);
			}
			if (UnityUIButton != nullptr)
			{
				UnityUIButton->PlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::Out);
			}
			if (UnityUIText != nullptr)
			{
				UnityUIText->PlayAnim(nNowPageNo(), MotosUIRuleText::EAnim::Out);
			}
			m_nStep++;
		}
		else
		{
			if (m_nStep != 1)
			{
				break;
			}
			bool flag = true;
			if (flag)
			{
				flag = pAnita(0u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(2u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(3u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(4u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(16u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(18u)->IsState(65536u);
			}
			if (flag)
			{
				flag = pAnita(17u)->IsState(65536u);
			}
			if (flag && pAnita(15u)->IsState(1u))
			{
				flag = pAnita(15u)->IsState(65536u);
			}
			if (nullptr != UnityUIButton)
			{
				if (flag)
				{
					flag = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::Close, MotosUIRuleButtonSet::EAnim::Out);
				}
				if (flag)
				{
					flag = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::NextPage, MotosUIRuleButtonSet::EAnim::Out);
				}
				if (flag)
				{
					flag = !UnityUIButton->IsPlayAnim(MotosUIRuleButtonSet::EButtonType::PrevPage, MotosUIRuleButtonSet::EAnim::Out);
				}
			}
			if (flag)
			{
				pAnita(0u)->SetState(1u, false);
				pAnita(2u)->SetState(1u, false);
				pAnita(3u)->SetState(1u, false);
				pAnita(4u)->SetState(1u, false);
				pAnita(16u)->SetState(1u, false);
				pAnita(18u)->SetState(1u, false);
				pAnita(17u)->SetState(1u, false);
				pAnita(15u)->SetState(1u, false);
				for (uint num = 0u; num < nMaxPageNo(); num++)
				{
					pAnita(_nGetOrgAntNo(num))->SetState(1u, false);
				}
				if (UnityUIButton != nullptr)
				{
					UnityUIButton->SetEnable(false);
				}
				if (UnityUIImage != nullptr)
				{
					UnityUIImage->SetEnable(false);
				}
				if (UnityUIText != nullptr)
				{
					UnityUIText->SetEnable(false);
				}
				m_nState = 6;
			}
		}
		break;
	}
	return cTaskAnita::Execute(pParam);
}

void cTaskConsMenuTutorialBase::Draw(int nStatus, cJetTask* pParam)
{
	cTaskAnita::Draw(nStatus, pParam);
	// DOL 0x80146604 state 1 (queues the page text every frame; 0x801477a4 draws the anita vector, then the writer at +0x214):
	// FUN_801394b4(csv +0x2c8, row +0x210 = page's nTextRow, column 0) -> FUN_80016e88 (writer mode 2 = flags 0x311) at
	// (X + 320, 240 - Y) where (X, Y) = layer position kept at +0x1f0 (page 0) or +0x200 (other pages): anita 0, layer 0, pattern 1 / 2 (stored by
	// FUN_80145cbc = RegistData, 0x80145f64 / 0x80145f80, FUN_80014408). 0x311 = nw4r VERTICAL_ORIGIN_BASELINE | centred block | centred lines.
	// Checked in Dolphin (g07 tutorial page 1): queue entry x 320, y 123 (= 240 - 117), colour 0x000000ff, mode 2, size 24.0 (writer +0xa8;
	// nw4r SetFontSize -> scale 24 / 32 = 0.75: the writer's font is RESIDENT_FONT_NOFRAME of resource.lzs (RFNT at 0x91f1f660, 0x99c5c bytes,
	// height 32, ascent 26), not the outlined FRAME font); the first line's baseline is at y.
	if (m_nState == 1 && pData() != nullptr)
	{
		const Vector4& pos = m_vFontPos[nNowPageNo() == 0u ? 0 : 1];
		{
			const std::string text = m_Text.Cell(nCsvRow(), 0);
			gfx::TextStyle st;
			st.font = "RESIDENT_FONT_NOFRAME";
			const float size = (float)std::atof(m_Text.Cell(0, 0).c_str());
			const int height = gfx::FontHeight(st.font);
			st.scaleX = st.scaleY = (size > 0 ? size : 24.0f) / (height > 0 ? height : 32);
			st.color = 0x000000ffu;
			st.vorigin = gfx::TextStyle::VOrigin::Baseline;
			gfx::DrawText(320.0f + pos.x, 240.0f - pos.y, text, st, gfx::TextAlign::Center);
		}
	}
}

bool cTaskConsMenuTutorialBase::IsVisibleButton(cTaskConsMenuTutorialBase::EAnitaID anita)
{
	MotosUIRuleButtonSet::EButtonType type;
	switch (anita)
	{
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_CLOSE:
		type = MotosUIRuleButtonSet::EButtonType::Close;
		break;
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT:
		type = MotosUIRuleButtonSet::EButtonType::NextPage;
		break;
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV:
		type = MotosUIRuleButtonSet::EButtonType::PrevPage;
		break;
	default:
		return false;
	}
	if (!pAnita((uint)anita)->IsState(1u))
	{
		if (nullptr != UnityUIButton)
		{
			return UnityUIButton->IsVisible(type);
		}
		return false;
	}
	return true;
}

bool cTaskConsMenuTutorialBase::IsButtonAnimEnd(cTaskConsMenuTutorialBase::EAnitaID anita)
{
	MotosUIRuleButtonSet::EButtonType type;
	switch (anita)
	{
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_CLOSE:
		type = MotosUIRuleButtonSet::EButtonType::Close;
		break;
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_NEXT:
		type = MotosUIRuleButtonSet::EButtonType::NextPage;
		break;
	case cTaskConsMenuTutorialBase::EAnitaID::eANT_PREV:
		type = MotosUIRuleButtonSet::EButtonType::PrevPage;
		break;
	default:
		return false;
	}
	if (pAnita((uint)anita)->IsState(65536u))
	{
		if (!(nullptr == UnityUIButton))
		{
			return !UnityUIButton->IsPlayAnim(type);
		}
		return true;
	}
	return false;
}

bool cTaskConsMenuTutorialBase::ButtonVisibleValue(bool visible)
{
	bool result = visible;
	if (nullptr != UnityUIButton)
	{
		result = false;
	}
	return result;
}

bool cTaskConsMenuTutorialBase::IsVisibleImage(uint pageNo)
{
	if (!pAnita(_nGetOrgAntNo(pageNo))->IsState(1u))
	{
		if (nullptr != UnityUIImage)
		{
			return UnityUIImage->IsVisible(pageNo);
		}
		return false;
	}
	return true;
}

bool cTaskConsMenuTutorialBase::ImageVisibleValue(bool visible)
{
	bool result = visible;
	if (nullptr != UnityUIImage)
	{
		result = false;
	}
	return result;
}

bool cTaskConsMenuTutorialBase::TextVisibleValue(bool visible)
{
	bool result = visible;
	if (nullptr != UnityUIImage)
	{
		result = false;
	}
	return result;
}

}
