#include "kingdom/sMatchingData.hpp"

#include <iterator>

namespace kingdom
{

sMatchingData::sMatchingData()
{
	for (int i = 0; i < (int)std::size(Info); i++)
	{
		Info[i] = new sMatchingData::InfoData();
	}
}

sMatchingData* sMatchingData::Clone()
{
	// FIXME(cs2cpp): unresolved identifier MemberwiseClone
	// TODO(unity): return (sMatchingData)MemberwiseClone();
	return new sMatchingData(*this);   // MemberwiseClone
}

}
