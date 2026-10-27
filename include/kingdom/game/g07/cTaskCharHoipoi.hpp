#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskChar.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g07
{

class cTaskCharHoipoi : public cTaskChar
{
public:
	enum EAnm
	{
		eANM_TSA = 0,
		eANM_CLA = 1,
		eANM_MAX = 2
	};

private:
	Vector4 m_vTarget{};

	float m_fSpeed[2]{};

	float m_fFrame{};

public:
	cTaskCharHoipoi();

	Vector4& vTarget();

private:
	void Init();

public:
	void Term() override;

	bool Create(uint no);

	void UpdateMatrix() override;

	bool Execute(cJetTask* pParam) override;

	bool AnimateModel(uint model) override;

	void InitFrame();

	void LoadFrameRate();

	void StopFrameRate();

private:
	void SaveFrameRate();

public:
	bool Move() override;

	bool ActionLocal() override;

	bool ActionOther() override;

protected:
	void ReleaseTerm() override { cTaskCharHoipoi::Term(); cTaskModelBase::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
