#include "kingdom/KingdomJob.hpp"

#include "KingdomGlobal.hpp"
#include "cSysCoreStructure.hpp"
#include "kingdom/cKingdomSys.hpp"

namespace kingdom
{

void Kingdom_SetJobNo(uint jobNo, bool force, bool fade)
{
	if (!force)
	{
		KingdomGlobal::SysCore->SetForceJobNo(jobNo);
		return;
	}
	KingdomGlobal::KingSys->nNextJob() = jobNo;
	KingdomGlobal::SysCore->SetJobNo(fade ? job::CHANGE_FADE : job::CHANGE);
}

}
