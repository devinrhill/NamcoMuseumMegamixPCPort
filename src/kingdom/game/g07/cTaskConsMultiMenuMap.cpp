#include "kingdom/game/g07/cTaskConsMultiMenuMap.hpp"

#include "CustomKingdomUtil.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g07
{

void cTaskConsMultiMenuMap::ChangeData(uint nWorldNo, uint nStageNo)
{
}

cTaskConsMultiMenuMap::cTaskConsMultiMenuMap(cTaskConsMenuManBase* const& pConsMan, uint nWorldNo)
	: cTaskConsMenuMap(pConsMan, nWorldNo, 0u)
{
}

cTaskConsMultiMenuMap::~cTaskConsMultiMenuMap()
{
}

void cTaskConsMultiMenuMap::ChangeData(uint nWorldNo)
{
	CustomKingdomUtil::Assert(nWorldNo < 10, "error");
	cTaskConsMenuMap::nWorldNo() = nWorldNo;
}

int cTaskConsMultiMenuMap::nGetMapAnm(uint nState)
{
	int obj[10][5] = {{175, 176, 177, 178, 179}, {180, 181, 182, 183, 184}, {185, 186, 187, 188, 189}, {190, 191, 192, 193, 194}, {195, 196, 197, 198, 199}, {200, 201, 202, 203, 204}, {205, 206, 207, 208, 209}, {210, 211, 212, 213, 214}, {215, 216, 217, 218, 219}, {220, 221, 222, 223, 224}};
	CustomKingdomUtil::Assert(nWorldNo() < 10, "error");
	return obj[nWorldNo()][nState];
}

}
