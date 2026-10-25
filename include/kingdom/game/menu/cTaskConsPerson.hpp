#pragma once

#include "common.hpp"
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "MotosUIMotosMultiHome.hpp"

namespace kingdom::game::menu
{

class cTaskConsPerson : public cTaskConsMatching
{
private:
	enum EAnt
	{
		eANT_PERSON = 0
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

public:
	enum EType
	{
		eTYPE_TWO = 0,
		eTYPE_THREE = 1,
		eTYPE_FOUR = 2,
		eTYPE_MAX = 3
	};

private:
	int HIT_PAT[3] = {7, 8, 9};

	uint m_nType{};

	int m_nHitPat{};

	MotosUIMotosMultiHome::EButtonType m_buttonType{};

public:
	MotosUIMotosMultiHome* UnityUI{};

protected:
	uint& nType();

	int& nHitPat();

public:
	virtual ~cTaskConsPerson();

	uint nGetType();

	cTaskConsPerson(int nId, string Key, uint nType);

protected:
	virtual int nGetPersonAnmNo(uint nState);

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
