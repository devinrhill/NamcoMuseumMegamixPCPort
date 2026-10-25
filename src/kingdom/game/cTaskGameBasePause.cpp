#include "kingdom/game/cTaskGameBasePause.hpp"

#include <limits>

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomAnita.hpp"
#include "kingdom/cKingdomAnitaVector.hpp"
#include "kingdom/Constants.hpp"
#include "kingdom/cTaskCursor.hpp"
#include "kingdom/cTaskFade.hpp"
#include "GamePad.hpp"
#include "kingdom/sMatchingData.hpp"
#include "MotosFrontUIManager.hpp"
#include "MotosUIPause.hpp"
#include <ESEKind.hpp>
#include <InGamePadManager.hpp>
#include <PadDefine.hpp>
#include <SoundManager.hpp>

namespace kingdom::game
{

cKingdomAnita* cTaskGameBasePause::pAnita(uint n)
{
	return AnitaVector->pAnita(n);
}

cTaskGameBasePause::cTaskGameBasePause()
{
	Init();
}

cTaskGameBasePause::cTaskGameBasePause(string key, string cursor, sMatchingData* pMatchingData)
{
	Init();
	CustomKingdomUtil::Assert(RegistData(key, cursor, pMatchingData), "error");
}

cTaskGameBasePause::~cTaskGameBasePause()
{
	Free();
}

void cTaskGameBasePause::Free()
{
	Term();
}

uint& cTaskGameBasePause::nMenuJob()
{
	return m_nMenuJob;
}

bool cTaskGameBasePause::IsPause()
{
	return m_bPause;
}

void cTaskGameBasePause::Init()
{
	AnitaVector = new cKingdomAnitaVector();
	nDrawPriority(2415919101u);
	m_nMenuJob = std::numeric_limits<uint>::max();
	m_nSelect = 0u;
	m_bPause = false;
	m_pMatchingData = nullptr;
	m_nCtrlNum = 0u;
	ConnectChild(m_pTaskFade = new cTaskFade());
	UnityUI = MotosFrontUIManager::Get<MotosUIPause>(MotosFrontUIManager::EUIIndex::Pause);
}

void cTaskGameBasePause::Term()
{
}

bool cTaskGameBasePause::RegistData(string key, string cursor, sMatchingData* pMatchingData)
{
	int drawLayer = 0; // TODO(unity): drawLayer = LayerMask.NameToLayer("FrontUI")
	for (uint num = 0u; num < 4; num++)
	{
		pAnita(num)->RegistData(key, drawLayer);
		pAnita(num)->SetState(3u, false);
	}
	// DOL 0x80147a64: cTaskMenuCursor x 4 (pad i, idle anim 2i, pressed anim 2i + 1 on A / B / Z = 0x860), hidden until the pause opens
	for (int i = 0; i < 4; i++)
	{
		m_pCursor[i] = new kingdom::cTaskCursor();
		m_pCursor[i]->Init(i, cursor.c_str(), i * 2);
		m_pCursor[i]->AddButton(0x860u, i * 2 + 1);
		m_pCursor[i]->nDrawPriority(0x8ffffffeu);
		m_pCursor[i]->SetEnable(false);
		if (pMatchingData != nullptr && pMatchingData->Info[i] != nullptr) m_pCursor[i]->SetPadNo((int)pMatchingData->Info[i]->nWork);
		ConnectChild(m_pCursor[i]);
	}
	if (pMatchingData != nullptr)
	{
		m_pMatchingData = pMatchingData;
		m_nCtrlNum = pMatchingData->nNum;
	}
	else
	{
		m_nCtrlNum = 1u;
	}
	return true;
}

void cTaskGameBasePause::ShowCursors(bool on)
{
	for (uint i = 0; i < 4; i++)
		if (m_pCursor[i] != nullptr) m_pCursor[i]->SetEnable(on && i < m_nCtrlNum);
}

bool cTaskGameBasePause::Start()
{
	if (m_bPause)
	{
		return false;
	}
	m_bPause = true;
	return true;
}

bool cTaskGameBasePause::Execute(cJetTask* pParam)
{
	int pad1No = InGamePadManager::GetPad1No();
	uint n = 0u;
	if (nModeState(n) == 0)
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch1_0u:
			nModeStep(n) = 1u;
			pAnita(0u)->SetState(1u, false);
			pAnita(1u)->SetState(1u, false);
			pAnita(2u)->SetState(1u, false);
			pAnita(3u)->SetState(1u, false);
			if (UnityUI != nullptr)
			{
				UnityUI->SetEnable(false);
			}
			if (UnityUI != nullptr)
			{
				UnityUI->ResetAllAnim();
			}
			SoundManager::get_SE()->Resume();
			m_bPause = false;
			goto _switch1_1u;
		case 1u:
		_switch1_1u:
			if (m_bPause)
			{
				NextMode(n, 1u);
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch1_2u;
		case 2u:
		_switch1_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (1 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch2_0u:
			{
				nModeStep(n) = 1u;
				pAnita(0u)->SetAnimNo(13);
				pAnita(0u)->SetState(3u);
				pAnita(0u)->IsHBMUpdate() = true;
				pAnita(3u)->SetAnimNo(0);
				pAnita(3u)->SetState(3u);
				pAnita(3u)->IsHBMUpdate() = true;
				bool bit = nullptr == UnityUI;
				pAnita(1u)->SetAnimNo(9);
				pAnita(1u)->SetState(3u, bit);
				pAnita(2u)->SetAnimNo(4);
				pAnita(2u)->SetState(3u, bit);
				if (UnityUI != nullptr)
				{
					UnityUI->SetEnable(true);
				}
				ShowCursors(true);   // DOL 0x80147cb0 state 1: +0x1e1 = 1 for each controller
				for (uint i = 0; i < m_nCtrlNum; i++) GamePad::Rumble((int)i, 0);   // 0x80147f80: every controller's motor stops
				SoundManager::get_SE()->Suspend();
				SoundManager::get_SE()->PlaySE(ESEKind::RESIDENT_SND_SE10_PAUSE);
				goto _switch2_1u;
			}
		case 1u:
		_switch2_1u:
			{
				if (pAnita(0u)->IsState(65536u))
				{
					pAnita(0u)->SetAnimNo(14);
				}
				if (pAnita(3u)->IsState(65536u))
				{
					pAnita(3u)->SetAnimNo(1);
				}
				bool num = pAnita(0u)->GetAnimNo() == 14;
				bool flag = pAnita(3u)->GetAnimNo() == 1;
				if (num)
				{
					pAnita(0u)->IsHBMUpdate() = false;
				}
				if (flag)
				{
					pAnita(3u)->IsHBMUpdate() = false;
				}
				if (num & flag)
				{
					NextMode(n, 2u);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch2_2u;
			}
		case 2u:
		_switch2_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (2 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch3_0u:
			nModeStep(n) = 1u;
			pAnita(0u)->SetAnimNo(14);
			pAnita(1u)->SetAnimNo(9);
			pAnita(2u)->SetAnimNo(4);
			if (UnityUI != nullptr)
			{
				UnityUI->ResetAnim(MotosUIPause::EButtonType::ReturnMenu);
			}
			if (UnityUI != nullptr)
			{
				UnityUI->ResetAnim(MotosUIPause::EButtonType::Continue);
			}
			pAnita(3u)->SetAnimNo(1);
			goto _switch3_1u;
		case 1u:
		_switch3_1u:
			if (InGamePadManager::GetEnterKeyDown(pad1No))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
				NextMode(n, 3u);
			}
			else if (InGamePadManager::GetKeyDown(pad1No, PadDefine::EPadKeycode::eKEY_START) || InGamePadManager::GetCancelKeyDown(pad1No))
			{
				SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE033_ITEM_);
				m_nSelect = 0u;
				NextMode(n, 3u);
			}
			else
			{
				uint nSelect = m_nSelect;
				uint num2 = 2u;
				float x = InGamePadManager::GetStick(pad1No, PadDefine::scLSTICK).x;
				x = ((0.5f < Mathf::Abs(x)) ? x : 0.0f);
				if (x < 0.0f || InGamePadManager::GetKeyDown(pad1No, PadDefine::EPadKeycode::eKEY_LEFT) || InGamePadManager::GetKeyDown(pad1No, PadDefine::EPadKeycode::eKEY_LSTICK_LEFT))
				{
					if (x <= 0.0f)
					{
						num2 = 0u;
					}
				}
				else if ((x > 0.0f || InGamePadManager::GetKeyDown(pad1No, PadDefine::EPadKeycode::eKEY_RIGHT) || InGamePadManager::GetKeyDown(pad1No, PadDefine::EPadKeycode::eKEY_RIGHT)) && x >= 0.0f)
				{
					num2 = 1u;
				}
				if (GamePad::Get(0).pointerCount > 0)   // DOL 0x80147cb0 state 2: the pointer over the continue (anita 2, pat 0) / menu (anita 1, pat 1) buttons
				{
					const GamePad::State& ps = GamePad::Get(0);
					const Vector4 pt(ps.pointerX * 640.0f * 0.5f, -ps.pointerY * 480.0f * 0.5f, 0.0f, 1.0f);
					if (pAnita(2u)->IsHit(pt, 0)) num2 = 0u;
					else if (pAnita(1u)->IsHit(pt, 1)) num2 = 1u;
				}
				if (num2 != 2)
				{
					m_nSelect = num2;
					if (m_nSelect != nSelect)
					{
						SoundManager::get_SE()->PlaySE(ESEKind::MOTOS_SND_SE032_ITEM_SELECT);
					}
				}
			}
			if (m_nSelect == 1)
			{
				pAnita(1u)->SetAnimNo(10, false);
				pAnita(2u)->SetAnimNo(4, false);
				if (nullptr != UnityUI && !UnityUI->IsPlayAnim(MotosUIPause::EButtonType::ReturnMenu, MotosUIPause::EAnim::Highlight))
				{
					if (UnityUI != nullptr)
					{
						UnityUI->PlayAnim(MotosUIPause::EButtonType::ReturnMenu, MotosUIPause::EAnim::Highlight);
					}
					if (UnityUI != nullptr)
					{
						UnityUI->ResetAnim(MotosUIPause::EButtonType::Continue);
					}
				}
			}
			else
			{
				pAnita(1u)->SetAnimNo(9, false);
				pAnita(2u)->SetAnimNo(5, false);
				if (nullptr != UnityUI && !UnityUI->IsPlayAnim(MotosUIPause::EButtonType::Continue, MotosUIPause::EAnim::Highlight))
				{
					if (UnityUI != nullptr)
					{
						UnityUI->PlayAnim(MotosUIPause::EButtonType::Continue, MotosUIPause::EAnim::Highlight);
					}
					if (UnityUI != nullptr)
					{
						UnityUI->ResetAnim(MotosUIPause::EButtonType::ReturnMenu);
					}
				}
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch3_2u;
		case 2u:
		_switch3_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (3 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch4_0u:
			nModeStep(n) = 1u;
			if (m_nSelect == 1)
			{
				pAnita(1u)->SetAnimNo(11);
				if (UnityUI != nullptr)
				{
					UnityUI->PlayAnim(MotosUIPause::EButtonType::ReturnMenu, MotosUIPause::EAnim::Push);
				}
			}
			else
			{
				pAnita(2u)->SetAnimNo(6);
				if (UnityUI != nullptr)
				{
					UnityUI->PlayAnim(MotosUIPause::EButtonType::Continue, MotosUIPause::EAnim::Push);
				}
			}
			goto _switch4_1u;
		case 1u:
		_switch4_1u:
			{
				uint num3 = ((m_nSelect == 1) ? 1u : 2u);
				if ((pAnita(num3)->IsState(327680u) && nullptr == UnityUI) || !UnityUI->IsPlayAnim((MotosUIPause::EButtonType)(2 - num3), MotosUIPause::EAnim::Push))
				{
					num3 = ((m_nSelect == 1) ? 5u : 4u);
					NextMode(n, num3);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch4_2u;
			}
		case 2u:
		_switch4_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (4 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch5_0u:
			nModeStep(n) = 1u;
			ShowCursors(false);
			pAnita(0u)->SetAnimNo(15);
			pAnita(1u)->SetAnimNo(12);
			pAnita(2u)->SetAnimNo(7);
			if (UnityUI != nullptr)
			{
				UnityUI->PlayAnim(MotosUIPause::EButtonType::ReturnMenu, MotosUIPause::EAnim::Out);
			}
			if (UnityUI != nullptr)
			{
				UnityUI->PlayAnim(MotosUIPause::EButtonType::Continue, MotosUIPause::EAnim::Out);
			}
			pAnita(3u)->SetAnimNo(2);
			goto _switch5_1u;
		case 1u:
		_switch5_1u:
			{
				bool num4 = pAnita(0u)->IsState(65536u);
				bool flag2 = (pAnita(1u)->IsState(65536u) && nullptr == UnityUI) || !UnityUI->IsPlayAnim(MotosUIPause::EButtonType::ReturnMenu);
				bool flag3 = (pAnita(2u)->IsState(65536u) && nullptr == UnityUI) || !UnityUI->IsPlayAnim(MotosUIPause::EButtonType::Continue);
				bool flag4 = pAnita(3u)->IsState(65536u);
				if (num4 & flag2 & flag3 & flag4)
				{
					NextMode(n, 0u);
				}
				if (nOldState(n) == nModeState(n))
				{
					break;
				}
				nModeStep(n) = 2u;
				goto _switch5_2u;
			}
		case 2u:
		_switch5_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	else if (5 == nModeState(n))
	{
		nOldState(n) = nModeState(n);
		switch (nModeStep(n))
		{
		case 0u:
		_switch6_0u:
			nModeStep(n) = 1u;
			ShowCursors(false);
			m_pTaskFade->Fade(65538u);
			goto _switch6_1u;
		case 1u:
		_switch6_1u:
			if (m_pTaskFade->IsEnd())
			{
				if (UnityUI != nullptr)
				{
					UnityUI->SetEnable(false);
				}
				if (UnityUI != nullptr)
				{
					UnityUI->ResetAllAnim();
				}
				SoundManager::get_SE()->Resume();
				CustomKingdomUtil::Assert(nMenuJob() != std::numeric_limits<uint>::max(), "error");
				Constants::Kingdom_SetJobNo(nMenuJob());
			}
			if (nOldState(n) == nModeState(n))
			{
				break;
			}
			nModeStep(n) = 2u;
			goto _switch6_2u;
		case 2u:
		_switch6_2u:
			nModeStep(n) = 0u;
			break;
		}
	}
	AnitaVector->UpdateFrame();
	return true;
}

void cTaskGameBasePause::Draw(int nStatus, cJetTask* pParam)
{
	cKingdomAnita::UpdateMatrix();
	AnitaVector->Draw();
}

}
