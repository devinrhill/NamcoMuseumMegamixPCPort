#include "kingdom/game/g07/cTaskConsMultiSetup.hpp"
#include "kingdom/game/g07/cMotosGameSys.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "kingdom/game/g07/cAppG07MotosGame.hpp"
#include "kingdom/game/g07/cTaskConsItemNumber.hpp"
#include "kingdom/game/g07/cTaskConsPacIconSingleSetting.hpp"
#include "kingdom/game/g07/cTaskConsPlayerNumber.hpp"
#include "kingdom/ResourceDatabase.hpp"
#include "KingdomGlobal.hpp"
#include "MotosUIManager.hpp"
#include "MotosUIMotosStartText.hpp"
#include <Constants.hpp>
#include "gfx/FontWriter.hpp"

namespace kingdom::game::g07
{

const uint cTaskConsMultiSetup::FONT_SIZE = 28u;
const float cTaskConsMultiSetup::HINT_POS[2] = {-158.0f, 104.0f};
const float cTaskConsMultiSetup::PLAYER_NO_POS1[2] = {65.0f, 184.0f};
const float cTaskConsMultiSetup::PLAYER_NO_POS2[2] = {68.0f, 89.0f};
const float cTaskConsMultiSetup::FONT_POS[2] = {160.0f, 54.0f};
const float cTaskConsMultiSetup::FONT_POS_WIN[2] = {0.0f, -70.0f};

cTaskConsMultiSetup::cTaskConsMultiSetup()
{
	Init();
}

cTaskAnita* cTaskConsMultiSetup::Construct()
{
	return new cTaskConsMultiSetup();
}

// DOL 0x80100e0c [audited: per AUDIT_G03_G07 row]
void cTaskConsMultiSetup::Init()
{
	if (KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		UnityUIStart = MotosUIManager::Get<MotosUIMotosStartText>(MotosUIManager::EUIIndex::MotosStartText);
		if (UnityUIStart != nullptr)
		{
			UnityUIStart->Init(MotosUIMotosStartText::EType::Multi);
		}
		m_pPacIcon = new cTaskConsPacIconSingleSetting();
		ConnectChild(m_pPacIcon);
		m_aExecNumber->Value = 0;
		int array[13] = {266, 127, 105, 112, 115, 109, 254, 277, 178, 181, 184, 187, 178};
		for (uint num = 0u; num < 13; num++)
		{
			int num2 = ::Constants::ARRAY_SIZE(array);
			CustomKingdomUtil::Assert((long)num < (long)num2, "error");
			pAnita(num)->RegistData(ResourceDatabase::GetPath("ANT_CONSOLE"));
			pAnita(num)->SetAnimNo(array[num]);
			pAnita(num)->SetState(3u, false);
		}
		m_pPlayerNo[0] = new cTaskConsPlayerNumber();
		CustomKingdomUtil::Assert(m_pPlayerNo[0] != nullptr, "error");
		pAnita(0u)->GetLayerPos(m_pPlayerNo[0]->vTrans(), 0u, 21);
		ConnectChild(m_pPlayerNo[0]);
		m_pPlayerNo[1] = new cTaskConsPlayerNumber();
		CustomKingdomUtil::Assert(m_pPlayerNo[0] != nullptr, "error");
		pAnita(0u)->GetLayerPos(m_pPlayerNo[1]->vTrans(), 0u, 22);
		ConnectChild(m_pPlayerNo[1]);
		Vector2 vector = Vector2(-122.0f, -135.0f);
		m_pItemNumber = new cTaskConsItemNumber(2);
		m_pItemNumber->X() = vector[0];
		m_pItemNumber->Y() = vector[1];
		m_pItemNumber->pExecNumber(m_aExecNumber);
		ConnectChild(m_pItemNumber);
		Vector4 pos = Vector4();
		pAnita(0u)->GetLayerPos(pos, 0u, 62);
		m_pRoundNumber = new cTaskConsItemNumber(2);
		m_pRoundNumber->pExecNumber(m_aExecRoundNumber);
		m_pRoundNumber->X() = pos[0];
		m_pRoundNumber->Y() = pos[1];
		m_pRoundNumber->SetFlag(6u, true);
		m_pRoundNumber->SetFlag(1u, false);
		ConnectChild(m_pRoundNumber);
	}
}

void cTaskConsMultiSetup::Term()
{
	if (nullptr != UnityUIStart && UnityUIStart->IsEnable())
	{
		if (UnityUIStart != nullptr)
		{
			UnityUIStart->SetEnable(false);
		}
	}
}

void cTaskConsMultiSetup::Setup()
{
}

// DOL 0x8010183c [audited: per AUDIT_G03_G07 row] (names: the 0x80016e88 calls of the setup / result branches)
bool cTaskConsMultiSetup::Execute(cJetTask* pParam)
{
	m_names.clear();
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay())
	{
		return cTaskConsoleBase::Execute(pParam);
	}
	if (KingdomGlobal::g_pMotosGame->nStep(0) == 1)
	{
		{
			// the adjusting player's name at (320 + 160, 240 - 54) in the player's colour (table 0x8036ec50), black shadow
			static const uint kColor[4] = {0xb4b4ffffu, 0x96ff96ffu, 0xe6ff32ffu, 0xffb4e6ffu};
			const uint p = KingdomGlobal::g_pMotosGame->nPlayerParam();
			const sMatchingData::InfoData* info = KingdomGlobal::MotosSys->pGetMatchingData()->Info[p];
			if (info != nullptr && p < 4) QueueName(320.0f + FONT_POS[0], 240.0f - FONT_POS[1], info->Name, kColor[p], 0xffu);
		}
		for (uint num = 0u; num < 2; num++)
		{
			m_pPlayerNo[num]->bExec() = true;
			m_pPlayerNo[num]->nPlayerNo() = KingdomGlobal::g_pMotosGame->nPlayerParam();
		}
		m_pItemNumber->bExec() = true;
		m_pRoundNumber->bExec() = true;
		int num2 = KingdomGlobal::pMotosGameSys->KeepData()[KingdomGlobal::g_pMotosGame->nPlayerParam()]->m_nUseItem[1] + KingdomGlobal::pMotosGameSys->KeepData()[KingdomGlobal::g_pMotosGame->nPlayerParam()]->m_nUseItem[2] + KingdomGlobal::pMotosGameSys->KeepData()[KingdomGlobal::g_pMotosGame->nPlayerParam()]->m_nUseItem[3];
		m_aExecNumber->Value = KingdomGlobal::pMotosGameSys->KeepData()[KingdomGlobal::g_pMotosGame->nPlayerParam()]->m_nItemNum[1] - num2;
		m_aExecRoundNumber->Value = KingdomGlobal::MotosSys->nForceVsRoundCount() - KingdomGlobal::MotosSys->nVsRoundCount() + 1;
		int array[4] = {254, 257, 260, 263};
		CustomKingdomUtil::Assert((long)KingdomGlobal::g_pMotosGame->nPlayerParam() < (long)::Constants::ARRAY_SIZE(array), "error");
		SetAnmDiffCurrent(6u, array[KingdomGlobal::g_pMotosGame->nPlayerParam()]);
		// DOL table 0x80376404: the index is clamped to 3 but the table has 3 entries; the DOL reads the next stack word (a value ~0 as a float)
		float array2[4] = {-84.0f, -136.0f, -188.0f, 0.0f};
		int num3 = KingdomGlobal::g_pMotosGame->nStep(1);
		if (num3 > 3)
		{
			num3 = 3;
		}
		pAnita(0u)->vTrans()[0] = 0.0f;
		pAnita(0u)->vTrans()[1] = array2[num3];
		int array3[3] = {211, 212, 213};
		if (num3 < 3) // index 3 reads the float -84.0 as an animation number in the DOL (UNVERIFIED what that draws): skipped
		{
			SetAnmDiffCurrent(1u, array3[num3]);
		}
		for (uint num4 = 0u; num4 < 8; num4++)
		{
			switch (num4)
			{
			case 2u:
				if (CustomKingdomUtil::IF(pAnita(num4)->GetState() & 1) && IsAnmEnd(num4))
				{
					pAnita(num4)->SetAnimNo(106);
				}
				break;
			case 3u:
				if (IsAnmEnd(num4))
				{
					SetAnm(num4, 113);
				}
				break;
			}
			Visible(num4);
		}
		if (nullptr != UnityUIStart)
		{
			m_pPlayerNo[0]->bExec() = false;
			if (UnityUIStart != nullptr)
			{
				UnityUIStart->SetPlayerNo((int)(KingdomGlobal::g_pMotosGame->nPlayerParam() + 1));
			}
		}
		if (nullptr != UnityUIStart && !UnityUIStart->IsEnable())
		{
			if (UnityUIStart != nullptr)
			{
				UnityUIStart->SetEnable(true);
			}
		}
	}
	else if (KingdomGlobal::g_pMotosGame->nStep(0) == 5)
	{
		m_pItemNumber->bExec() = false;
		m_pRoundNumber->bExec() = false;
		if (KingdomGlobal::g_pMotosGame->nMultiBattleResult() != 2)
		{
			int num5 = KingdomGlobal::g_pMotosGame->n1stPointPlayer();
			// the winner's name at (320 + 0, 240 + 70), white, black shadow
			if (const sMatchingData::InfoData* info = KingdomGlobal::MotosSys->pGetMatchingData()->Info[num5])
				QueueName(320.0f + FONT_POS_WIN[0], 240.0f - FONT_POS_WIN[1], info->Name, 0xffffffffu, 0xffu);
			for (uint num6 = 0u; num6 < 4; num6++)
			{
				uint set = num6 + 8;
				Invisible(set);
			}
			int array4[4] = {178, 181, 184, 187};
			Visible(12u);
			pAnita(12u)->vTrans()[0] = 0.0f;
			pAnita(12u)->vTrans()[1] = -30.0f;
			SetAnmDiffCurrent(12u, array4[num5]);
		}
		else
		{
			Invisible(12u);
			for (uint num7 = 0u; num7 < 4; num7++)
			{
				uint set2 = num7 + 8;
				Invisible(set2);
			}
			for (uint num8 = 0u; num8 < KingdomGlobal::MotosSys->pGetMatchingData()->nNum; num8++)
			{
				int drawPlayer = KingdomGlobal::g_pMotosGame->GetDrawPlayer(num8);
				if (drawPlayer != cAppG07MotosGame::DRAW_PLAYER_NONE)
				{
					Vector4 pos = Vector4();
					pAnita(0u)->GetLayerPos(pos, 0u, (int)(53 + num8));
					// each tied player's name at layer 0 pattern 53 + i (screen = 320 + x, 240 - y), white, black shadow
					if (const sMatchingData::InfoData* info = KingdomGlobal::MotosSys->pGetMatchingData()->Info[drawPlayer])
						QueueName(320.0f + pos[0], 240.0f - pos[1], info->Name, 0xffffffffu, 0xffu);
					uint num9 = (uint)(drawPlayer + 8);
					pAnita(0u)->GetLayerPos(pos, 0u, (int)(48 + num8));
					pAnita(num9)->vTrans()[0] = pos[0];
					pAnita(num9)->vTrans()[1] = pos[1];
					Visible(num9);
				}
			}
		}
		if (nullptr != UnityUIStart && UnityUIStart->IsEnable())
		{
			if (UnityUIStart != nullptr)
			{
				UnityUIStart->SetEnable(false);
			}
		}
	}
	else
	{
		m_pItemNumber->bExec() = false;
		m_pRoundNumber->bExec() = false;
		for (uint num10 = 0u; num10 < 2; num10++)
		{
			m_pPlayerNo[num10]->bExec() = false;
		}
		for (uint num11 = 0u; num11 < 13; num11++)
		{
			if (!CustomKingdomUtil::IF(pAnita(num11)->GetState() & 1))
			{
				continue;
			}
			uint array5[12] = {268u, 129u, 108u, 114u, 117u, 111u, 256u, 279u, 180u, 183u, 186u, 189u};
			int num12 = ::Constants::ARRAY_SIZE(array5);
			CustomKingdomUtil::Assert((long)num11 < (long)num12, "error");
			if (num11 == 6)
			{
				uint array6[4] = {256u, 259u, 262u, 265u};
				uint num13 = KingdomGlobal::MotosSys->pGetMatchingData()->nNum - 1;
				CustomKingdomUtil::Assert((long)num13 < (long)::Constants::ARRAY_SIZE(array6), "error");
				if ((long)pAnita(num11)->GetAnimNo() != (long)array6[num13])
				{
					pAnita(num11)->SetAnimNo((int)array6[num13]);
				}
			}
			// anita 12 (the winner crown): the DOL's table has 12 entries and reads the stack word after it for index 12 (the 0x43300000
			// double-conversion constant, UNVERIFIED what that does); skipped here
			else if (num11 < 12 && (long)pAnita(num11)->GetAnimNo() != (long)array5[num11])
			{
				pAnita(num11)->SetAnimNo((int)array5[num11]);
			}
			if (CustomKingdomUtil::IF(pAnita(num11)->GetState() & 0x10000))
			{
				pAnita(num11)->SetState(3u, false);
			}
		}
		if (nullptr != UnityUIStart && UnityUIStart->IsEnable())
		{
			if (UnityUIStart != nullptr)
			{
				UnityUIStart->SetEnable(false);
			}
		}
	}
	return cTaskConsoleBase::Execute(pParam);
}

// the DOL queues the shadow (colour `shadow`) at (x, y), then the text at (x - 1, y - 1); both truncated to int
void cTaskConsMultiSetup::QueueName(float x, float y, const std::string& text, uint color, uint shadow)
{
	if (text.empty()) return;
	m_names.push_back({(float)(int)x, (float)(int)y, text, shadow});
	m_names.push_back({(float)((int)x - 1), (float)((int)y - 1), text, color});
}

// DOL 0x80102190 [audited: per AUDIT_G03_G07 row]: the anitas, then (multiplayer, sys +0xcd8) the font writer queue
void cTaskConsMultiSetup::Draw(int nState, cJetTask* pParam)
{
	cTaskAnita::Draw(nState, pParam);
	if (!KingdomGlobal::g_pMotosGame->bMultiPlay() || m_names.empty()) return;
	gfx::TextStyle st;
	st.font = "RESIDENT_FONT_MII";   // UNVERIFIED: KingSys +0xa68 (as the other ports of that font slot)
	const int height = gfx::FontHeight(st.font);
	st.scaleX = st.scaleY = (float)FONT_SIZE / (float)(height > 0 ? height : 32);
	st.vorigin = gfx::TextStyle::VOrigin::Baseline;
	for (const sName& n : m_names)
	{
		st.color = n.color;
		gfx::DrawText(n.x, n.y, n.text, st, gfx::TextAlign::Center);
	}
}

}
