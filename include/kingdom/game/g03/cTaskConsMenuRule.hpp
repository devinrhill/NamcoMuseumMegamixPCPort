#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "MotosUIRollHome.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g03
{

class cTaskConsMenuRule : public cTaskConsMenuBase
{
private:
	enum EAnita
	{
		eANITA_MULTI = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3
	};

	static const int ANM_LIST[4];

	MotosUIRollHome::EButtonType m_buttonType{};

public:
	MotosUIRollHome* UnityUI{};

	cTaskConsMenuRule(cTaskConsMenuManBase* pConsMan);

	virtual ~cTaskConsMenuRule();

	bool IsDpdHit(float x, float y) override;

protected:
	void WaitExec() override;

	bool AppearExec() override;

	void PointExec() override;

	bool DecideExec() override;
};

}
