#include "kingdom/game/g07/MO_CHAR_PARAMS.hpp"

namespace kingdom::game::g07
{

MO_CHAR_PARAMS* MO_CHAR_PARAMS::Clone()
{
	// FIXME(cs2cpp): unresolved identifier MemberwiseClone
	return new MO_CHAR_PARAMS(*this);   // MemberwiseClone
}

}
