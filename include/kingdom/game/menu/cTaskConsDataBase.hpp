#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

class cTaskConsDataBase : public cTaskConsMatching
{
protected:
	enum EAnt
	{
		eANT_DATABASE = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_SELECT_WAIT = 2,
		eANM_POINT = 3,
		eANM_SELECT_POINT = 4,
		eANM_DECIDE = 5,
		eANM_DELETE = 6,
		eANM_MAX = 7
	};

	static int m_nSelectDataBase;

	virtual int nGetDataBaseAnmNo(uint nState);

	int& nSelectDataBase();

public:
	virtual ~cTaskConsDataBase();

	static int nGetSelectDataBase();

	static void nSetSelectDataBase(int nSelect);

	cTaskConsDataBase(int nId, string Key);

protected:
	bool AppearProc() override;

	void WaitProc() override;

	void PointProc() override;

	bool DecideProc() override;

	bool DeleteProc() override;
};

}
