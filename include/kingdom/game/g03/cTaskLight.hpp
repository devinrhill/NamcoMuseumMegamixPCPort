#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskLight : public cTaskKingdom
{
public:
	enum EType
	{
		eTYPE_NONE = 0,
		eTYPE_FOLLOW = 1,
		eTYPE_MAX = 2
	};

private:
	uint m_nType{};

	Vector4 m_vOffset{};

public:
	virtual ~cTaskLight();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

protected:
	void ReleaseTerm() override { cTaskLight::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
