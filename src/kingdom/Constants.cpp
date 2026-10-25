#include "kingdom/Constants.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/cKingdomSys.hpp"
#include "KingdomGlobal.hpp"

namespace kingdom
{

void Constants::Kingdom_System_Init()
{
	CustomKingdomUtil::Assert(KingdomGlobal::KingSys == nullptr, "fatal error");
	KingdomGlobal::KingSys = new cKingdomSys();
}

void Constants::Kingdom_System_Term()
{
	CustomKingdomUtil::Assert(KingdomGlobal::KingSys != nullptr, "fatal error");
	KingdomGlobal::KingSys = nullptr;
}

cKingdomSys* Constants::Kingdom_System()
{
	return KingdomGlobal::KingSys;
}

void Constants::Kingdom_SetJobNo(uint job, bool save, bool load)
{
	CustomKingdomUtil::Assert(job != 140509184, "error");
	CustomKingdomUtil::Assert(job != 139460608, "error");
	KingdomGlobal::SysCore->SetForceJobNo(job);
}

bool Constants::IS_PAUSE()
{
	return KingdomGlobal::KingSys->pAppMain()->IsPause();
}

}
