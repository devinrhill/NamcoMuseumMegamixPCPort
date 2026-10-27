#pragma once

#include "common.hpp"
#include "kingdom/cTaskKingdom.hpp"
#include "cJetTask.hpp"

namespace kingdom::game::g03
{

class cTaskFallArea : public cTaskKingdom
{
public:
	class sFallAreaArea
	{
	public:
		Vector4 vTrans{};

		float fLengthX{};

		float fLengthZ{};

		uint nAreaCheckKind{};
	};

	enum EType
	{
		eTYPE_RELATIVE = 0,
		eTYPE_ABSOLUTELY = 1,
		eTYPE_MAX = 2
	};

private:
	std::vector<cTaskFallArea::sFallAreaArea*> m_vecTaskFallArea;

public:
	cTaskFallArea();

	virtual ~cTaskFallArea();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;

	bool AreaCheck(Vector4 trans, Vector4 last);

protected:
	void ReleaseTerm() override { cTaskFallArea::Term(); cTaskKingdom::ReleaseTerm(); }   // DOL dtor chain (cTaskKingdom::TermOnce)
};

}
