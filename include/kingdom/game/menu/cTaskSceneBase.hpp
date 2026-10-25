#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include <ESEKind.hpp>
#include "kingdom/game/menu/cTaskConsMatching.hpp"
#include "kingdom/sMatchingData.hpp"

namespace kingdom::game::menu
{

class cTaskSceneBase : public cTaskKingdom
{
public:
	enum EKey
	{
		eUP = 0,
		eDOWN = 1,
		eLEFT = 2,
		eRIGHT = 3
	};

	static constexpr uint CTRL_CHECK_COUNT = 100u;

private:
	int m_nId{};

	int m_nState{};

	int m_nStep{};

	int m_nSelectNow{};

	int m_nSelectOld{};

	sMatchingData* m_pMatchingData{};

	std::vector<cTaskConsMatching*> m_vecHit;

protected:
	virtual void AllWait();

	virtual void KeySelect(const int& padNum);

	virtual void SelectProc(const int& padNum);

	virtual void DecideCheck(const int& padNum);

	int& nId();

	int& nState();

	int& nStep();

	int& nSelectNow();

	int& nSelectOld();

	sMatchingData*& pMatchingData();

	std::vector<cTaskConsMatching*>& vecHit();

public:
	cTaskSceneBase(int nId);

	virtual ~cTaskSceneBase();

	virtual void Reset();

	virtual void Appear();

	virtual bool IsAppearEnd();

	virtual void Wait();

	virtual void Delete();

	virtual bool IsDeleteEnd();

	virtual bool ErrorCheck();

	int nGetId();

	int nGetConnectCtrl(int nPlayerNo);

	int nGetConnectCtrlCt();

	void nSetMessageIdx(uint nIdx);

	void nSetErrorCode(uint nIdx);

protected:
	cTaskConsMatching* pGetHitObj(const int& nId);

public:
	virtual bool DpdSelect(const int& padNum);

	virtual void Select();

	static void PlaySndSe(const ESEKind& pSndName, int ctrl = 0);

	bool GetPadTrg(int padNum, cTaskSceneBase::EKey key);
};

}
