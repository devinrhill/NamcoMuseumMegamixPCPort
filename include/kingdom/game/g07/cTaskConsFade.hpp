#pragma once

#include "common.hpp"
#include "kingdom/game/g07/cTaskConsoleBase.hpp"
#include "cJetTask.hpp"
#include "kingdom/cTaskAnita.hpp"

namespace kingdom::game::g07
{

class cTaskConsFade : public cTaskConsoleBase
{
public:
	enum EAnm
	{
		eANM_MAIN = 0,
		eANM_MAX = 1
	};

	cTaskConsFade();

	virtual ~cTaskConsFade();

	void Free() override;

	static cTaskAnita* Construct();

private:
	void Init();

	void Term();

public:
	bool Execute(cJetTask* pParam) override;

	void Draw(int nStatus, cJetTask* pParam) override;
};

}
