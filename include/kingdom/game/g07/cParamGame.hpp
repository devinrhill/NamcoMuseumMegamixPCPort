#pragma once

#include "common.hpp"
#include "kingdom/cKingdomCSV.hpp"

namespace kingdom::game::g07
{

class cParamGame : public cKingdomCSV
{
private:
	enum ECSV
	{
		eCSV_MAX = 0
	};

protected:
	void Init() override;

	void Term() override;

private:
	void LoadValue();

public:
	cParamGame();

	cParamGame(string key);

	virtual ~cParamGame();

	bool RegistData(string key);
};

}
