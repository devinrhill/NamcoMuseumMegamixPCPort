#pragma once

#include "common.hpp"
#include "kingdom/game/cTaskConsMenuBase.hpp"
#include "kingdom/game/cTaskConsMenuManBase.hpp"

namespace kingdom::game::g01
{

// Galaga stage select: the world title plate (layout ANT_MENU, crc 0xd3a909fe). vtable 0x80437fa0; only Wait / Appear are
// overridden (Point / Decide / Delete are the base ones). Created by the console manager ctor (0x800bc114) as World(man),
// followed by SetWorldNo(world man+0x218) (0x800bfecc).
class cTaskConsMenuWorld : public cTaskConsMenuBase
{
protected:
	enum EAnm
	{
		eANM_APPEAR = 0,
		eANM_WAIT = 1,
		eANM_MAX = 2
	};

	static const int ANM_LIST[3][eANM_MAX];   // DOL 0x80370610: [world][anm]

private:
	int m_nWorldNo{};   // +0x1d8 (-1 until SetWorldNo)

public:
	cTaskConsMenuWorld(cTaskConsMenuManBase* const& pConsMan);   // DOL 0x800bfdc4

	virtual ~cTaskConsMenuWorld();   // DOL 0x800bfe74 (thunk 0x800c003c)

	void SetWorldNo(int nWorldNo);   // DOL 0x800bfecc (unnamed in the maps)

protected:
	void WaitExec() override;     // DOL 0x800bfed4

	bool AppearExec() override;   // DOL 0x800bff58
};

}
