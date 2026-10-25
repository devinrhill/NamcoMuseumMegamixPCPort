#pragma once

#include "common.hpp"
#include "kingdom/cTaskAnita.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskConsoleBase : public cTaskAnita
{
private:
	Vector4 m_vTrans{};

	int m_nState{};

	int m_nStateOld{};

	int m_nStep{};

	int m_nStepOld{};

	bool m_bStepChange{};

	int m_nApearAnim{};

	int m_nLoopAnim{};

	int m_nEndAnim{};

public:
	static int ANIM_NOT_USED;

	cTaskConsoleBase();

	virtual ~cTaskConsoleBase();

	virtual void Free();

	Vector4& vTrans();

	float& X();

	float& Y();

	int& nApearAnm();

	int& nLoopAnm();

	int& nEndAnm();

	int nStep();

	void StepChange(int set);

	bool InitStep();

	bool UpdateStep();

	bool EndStep();

	virtual bool IsStateChange();

	virtual void SetAnm(uint set, int nAnmNo);

	virtual bool SetAnmDiffCurrent(uint set, int nAnmNo);

	virtual bool IsAnmEnd(uint set);

	virtual void AnimationLoopFromApear(uint set);

	virtual void AnimationLoopFromApear(uint set, int nLoopAnimation);

private:
	void Init();

	void Term();

public:
	virtual void Create();

	virtual void Visible(uint set);

	virtual void Invisible(uint set);

	virtual bool IsAnmNoSame(uint set, int nAnmNo);

	bool Execute(cJetTask* pParam) override;

	bool BaseExecute(cJetTask* pParam);

	void BaseDraw(int nStatus, cJetTask* pParam);
};

}
