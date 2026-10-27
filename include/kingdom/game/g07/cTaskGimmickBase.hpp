#pragma once

#include "common.hpp"
#include "kingdom/cTaskModel.hpp"
#include "cJetTask.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"

namespace kingdom::game::g07
{

class cTaskGimmickBase : public cTaskModel
{
protected:
	uint m_nFlag{};

	uint m_nTaskKind{};

	int m_nStep{};

	uint m_nMode{};

	int m_nLCounter{};

	uint m_nMapPos[2]{};

	Vector4 m_vWTrans{};

	Vector4 m_vMTrans{};

	Vector4 m_vRot{};

	int m_nAnimeKick{};

	int m_nComaPattern{};

	int m_nGmkWidth{};

	int m_nGmkDepth{};

	int m_nComa{};

	uint m_nComaNum{};

	int m_nModel{};

	uint m_nHitChar{};

	uint m_nHitCounter{};

	uint m_nTimer[4]{};

	uint m_nWork[4]{};

public:
	uint& nFlag();

	uint& nTaskKind();

	int& nStep();

	uint& nMode();

	int& nLCounter();

	int& nComa();

	uint& nComaNum();

	int& nModel();

	uint& nMapPos(uint n);

	Vector4& vMTrans();

	Vector4& vRot();

	int& nGmkWidth();

	int& nGmkDepth();

	uint& nWork(int n);

	uint& nTimer(int n = 0);

	uint& nHitChar();

	uint& nHitCounter();

	virtual bool IsFall(Vector4 pMTrans);

	virtual bool IsAction(cTaskChar* pChar);

	virtual bool Create(uint no);

	cTaskGimmickBase();

private:
	void Init();

public:
	void Term();   // virtual in the DOL; the derived gimmicks call it, cTaskMap calls it from outside

protected:

private:

public:
	void UpdateFrame() override;

	Vector4& vWTrans();

	virtual void PresetEnv();

	virtual bool IsStepOnCntW(Vector4 pWTrans);

	virtual bool IsStepOnCnt(cTaskChar* pChar);

	void Draw(int nStatus, cJetTask* pParam) override;

	virtual bool IsStepOnTrig(cTaskChar* pChar);

	Vector4& vRotate();

protected:
	void ReleaseTerm() override { cTaskGimmickBase::Term(); cTaskModel::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
