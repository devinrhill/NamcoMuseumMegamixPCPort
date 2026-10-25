#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"

namespace kingdom::game::menu
{

class cTaskConsString : public cTaskConsMatching
{
protected:
	enum EAnt
	{
		eANT_STRING = 0
	};

	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_DELETE = 2,
		eANM_MAX = 3
	};

public:
	enum EString
	{
		eSTRING_PRESS_AB = 0,
		eSTRING_SELECT_MII = 1,
		eSTRING_QUESTION_OK = 2,
		eSTRING_MAX = 3
	};

private:
	int m_nStringIdx{};

protected:
	int& nStringIdx();

public:
	virtual ~cTaskConsString();

	void nSetStringIdx(int nIdx);

	int nGetStringIdx();

	cTaskConsString(int nId, string Key);

protected:
	virtual int nGetStringAnmNo(int nState);

	void WaitProc() override;

	bool AppearProc() override;

	bool DeleteProc() override;
};

}
