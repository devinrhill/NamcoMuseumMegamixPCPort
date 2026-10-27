#pragma once

#include "common.hpp"
#include "kingdom/KingdomJobTable.hpp"   // the job numbers (kingdom::job::*)

namespace kingdom
{

// DOL 0x8000db5c. force == false: switch immediately. Otherwise remember `jobNo` in cKingdomSys and go
// through the transition app (fade ? job::LOAD : job::SAVE), which jumps to the remembered job when it is done.
void Kingdom_SetJobNo(uint jobNo, bool force, bool fade);

}
