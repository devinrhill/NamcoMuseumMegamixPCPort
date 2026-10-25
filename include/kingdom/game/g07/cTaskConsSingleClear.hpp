#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsSingleClear : public cTaskConsoleBase
{
private:
	enum EAnm
	{
		eANM_CLEAR = 0,
		eANM_MAX = 1
	};

	enum EStep
	{
		eSTEP_INIT = 0,
		eSTEP_VISIBLE = 1,
		eSTEP_WAIT = 2,
		eSTEP_INVISIBLE = 3,
		eSTEP_END = 4
	};

	int m_nTimer{};

public:
	cTaskConsSingleClear();

	virtual ~cTaskConsSingleClear();

	void Free() override;

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;
};

}
