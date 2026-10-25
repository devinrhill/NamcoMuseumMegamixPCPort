#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "kingdom/game/g03/Constants.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g03/cTaskBgGimmick.hpp"

namespace kingdom::game::g03
{

class cTaskLandAttack : public cTaskKingdom
{
public:
	enum EAttack
	{
		ATTACK_NONE = 0,
		ATTACK_PLAY_ANM = 1,
		ATTACK_SEND_MSG = 2,
		ATTACK_ANMEND_WAIT = 3,
		ATTACK_IS_END = 4,
		ATTACK_LOOP = 5
	};

protected:
	cTaskBgGimmick* m_pGimmick{};

	bool m_nMesFlag{};

	uint m_nMesData{};

	int m_nFrameCount{};

	int m_nAttackKind{};

	int m_nAttackStep{};

	const Constants::LAND_ATTACK_INFO* const* const* m_ppAttackTBL{};

public:
	void Draw(int nStatus, cJetTask* pParam) override;

	cTaskLandAttack(cJetTask* pParent, cTaskBgGimmick* pGimmick);

	virtual ~cTaskLandAttack();

	bool Execute(cJetTask* pParam) override;

	bool IsAttackEnd();

	bool GetMsg(uint& pMes);

	void Pause(bool nFlag);

	void SetAttackTBL(const Constants::LAND_ATTACK_INFO* const* const* ppAtatckTBL);

	void SetAttackKind(int nKind);
};

}
