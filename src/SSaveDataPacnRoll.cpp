#include "SSaveDataPacnRoll.hpp"

SSaveDataPacnRoll::sStageRecord SSaveDataPacnRoll::sStageRecord::Clone()
{
	return *this;
}

void SSaveDataPacnRoll::Init()
{
	*this = SSaveDataPacnRoll();
	StageRecord.resize(26);
}
