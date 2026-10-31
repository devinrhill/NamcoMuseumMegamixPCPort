#include "kingdom/game/g03/cTaskLandAttack.hpp"
#include "kingdom/game/g03/cTaskCharPacman.hpp"

#include "cJetTask.hpp"
#include "CustomKingdomUtil.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"

namespace kingdom::game::g03
{

// DOL 0x8011f904 [audited: blr] (blr)
void cTaskLandAttack::Draw(int nStatus, cJetTask* pParam)
{
}

// DOL 0x8011f428 [audited: per AUDIT_G03_G07 row] (members: gimmick +0x1b4, mesFlag +0x1b8, mesData +0x1bc, frameCount +0x1c0, kind +0x1c4, step +0x1c8, table +0x1cc; table rows are 0xc bytes: frame, kind, param)
cTaskLandAttack::cTaskLandAttack(cJetTask* pParent, cTaskBgGimmick* pGimmick)
{
	SetName("cTaskLandAttack");
	m_pGimmick = pGimmick;
	m_pGimmick->nActFlag() |= 2147483648u;
	if (pParent != nullptr)
	{
		pParent->ConnectChild(this);
	}
	m_nMesFlag = false;
	m_nMesData = 0u;
	m_nFrameCount = 0;
	m_nAttackKind = -1;
	m_nAttackStep = 0;
	m_ppAttackTBL = nullptr;
}

// DOL 0x8011f59c [audited: per AUDIT_G03_G07 row]
cTaskLandAttack::~cTaskLandAttack()
{
}

// DOL 0x8011f600 [audited: per AUDIT_G03_G07 row]
bool cTaskLandAttack::Execute(cJetTask* pParam)
{
	if (!Constants::IS_PAC_PAUSE() && !Constants::PACMAN_OBJ()->IsMiss() && !Constants::IS_NAKAJIMA_PAUSE(this))
	{
		if (m_ppAttackTBL != nullptr && m_nAttackKind != -1)
		{
			bool flag = false;
			while (!flag && (!CustomKingdomUtil::IF(m_ppAttackTBL[m_nAttackKind][m_nAttackStep]->m_nFrame) || (long)m_ppAttackTBL[m_nAttackKind][m_nAttackStep]->m_nFrame <= (long)m_nFrameCount))
			{
				const Constants::LAND_ATTACK_INFO* lAND_ATTACK_INFO = m_ppAttackTBL[m_nAttackKind][m_nAttackStep];
				switch (lAND_ATTACK_INFO->m_nKind)
				{
				case 1u:
					m_nFrameCount = 0;
					m_pGimmick->Model->RegistAnm(0u, lAND_ATTACK_INFO->m_pStrParam);
					m_pGimmick->nActFlag() &= 2147483647u;
					m_nAttackStep++;
					break;
				case 2u:
					m_nMesFlag = true;
					m_nMesData = lAND_ATTACK_INFO->m_pUintParam;
					m_nAttackStep++;
					break;
				case 3u:
					if (CustomKingdomUtil::IF(m_pGimmick->nActFlag() & 0x80000000u))
					{
						m_nAttackStep++;
					}
					else
					{
						flag = true;
					}
					break;
				case 4u:
					flag = true;
					break;
				case 5u:
					m_nAttackStep = 0;
					m_nFrameCount = 0;
					break;
				}
			}
		}
		m_nFrameCount++;
	}
	// DOL 0x8011f600: the attack clip ends at "frame >= max - rate" (0x8001c2e0 / 0x8001c630 / 0x8001d790). The converted "frame >= max" never fired for the
	// looping w4sb clips (the frame wraps first): the centre platform repeated its first attack forever
	if (!CustomKingdomUtil::IF(m_pGimmick->nActFlag() & 0x80000000u) && m_pGimmick->Model->GetFrameAnmChr(0u) >= m_pGimmick->Model->GetFrameMaxAnmChr(0u) - m_pGimmick->Model->GetUpdateRateAnmChr(0u))
	{
		m_pGimmick->nActFlag() |= 2147483648u;
	}
	return true;
}

// DOL 0x8011f844 [audited: per AUDIT_G03_G07 row] (nActFlag >> 31)
bool cTaskLandAttack::IsAttackEnd()
{
	if (CustomKingdomUtil::IF(m_pGimmick->nActFlag() & 0x80000000u))
	{
		return true;
	}
	return false;
}

// DOL 0x8011f854 [audited: per AUDIT_G03_G07 row]
bool cTaskLandAttack::GetMsg(uint& pMes)
{
	if (m_nMesFlag)
	{
		pMes = m_nMesData;
		m_nMesFlag = false;
		return true;
	}
	return false;
}

// DOL 0x8011f880 [audited: per AUDIT_G03_G07 row] (nActFlag bit 0x40000000)
void cTaskLandAttack::Pause(bool nFlag)
{
	if (nFlag)
	{
		m_pGimmick->nActFlag() |= 1073741824u;
	}
	else
	{
		m_pGimmick->nActFlag() &= 3221225471u;
	}
}

// DOL 0x8011f8b0 [audited: per AUDIT_G03_G07 row]
void cTaskLandAttack::SetAttackTBL(const Constants::LAND_ATTACK_INFO* const* const* ppAtatckTBL)
{
	m_ppAttackTBL = ppAtatckTBL;
}

// DOL 0x8011f8b8 [audited: per AUDIT_G03_G07 row]
void cTaskLandAttack::SetAttackKind(int nKind)
{
	if (nKind == m_nAttackKind)
	{
		if (m_ppAttackTBL[m_nAttackKind][m_nAttackStep]->m_nKind == 4)
		{
			m_nAttackStep++;
		}
	}
	else
	{
		m_nAttackKind = nKind;
		m_nAttackStep = 0;
	}
}

}
