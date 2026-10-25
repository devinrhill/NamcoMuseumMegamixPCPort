#include "kingdom/game/menu/cTaskConsBg.hpp"

namespace kingdom::game::menu
{

cTaskConsBg::~cTaskConsBg()
{
}

cTaskConsBg::cTaskConsBg(int nId, string Key)
	: cTaskConsMatching(nId)
{
	nState() = 3;
	pAnita(0u)->RegistData(Key);
	pAnita(0u)->SetAnimNo(159);
	pAnita(0u)->SetState(1u);
}

}
