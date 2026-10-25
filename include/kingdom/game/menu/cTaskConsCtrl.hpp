#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

class cTaskConsCtrl : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_CONTROLLER = 0
	};

protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

private:
	uint m_nCtrlNo{};

	uint m_nPlayerNo{};

	bool m_bActive{};

public:
	MotosUIMotosMultiHome* UnityUI{};

protected:
	uint& nCtrlNo();

	uint& nPlayerNo();

	bool& bActive();

public:
	virtual ~cTaskConsCtrl();

	void nSetCtrlNo(uint nNo);

	uint nGetCtrlNo();

	void bActive(bool b);

	bool IsActive();

	cTaskConsCtrl(int nId, string Key, uint nPlayerNo);

protected:
	virtual int nGetCtrlAnmNo(uint nState);

	bool AppearProc() override;

	void WaitProc() override;

	bool DeleteProc() override;
};

}
