#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

class cTaskConsBack : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_BACK = 0
	};

protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_POINT = 2,
		eANM_DECIDE = 3,
		eANM_DELETE = 4,
		eANM_MAX = 5
	};

private:
	MotosUIMotosMultiHome::EButtonType m_buttonType{};

public:
	MotosUIMotosMultiHome* UnityUI{};

	virtual ~cTaskConsBack();

	cTaskConsBack(int nId, string Key);

protected:
	virtual int nGetBackAnmNo(uint nState);

public:
	bool IsDpdHit(float x, float y) override;

protected:
	bool AppearProc() override;

	void WaitProc() override;

	void PointProc() override;

	bool DecideProc() override;

	bool DeleteProc() override;
};

}
