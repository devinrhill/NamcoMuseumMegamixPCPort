#include "kingdom/game/g04/g04Util.hpp"

#include "kingdom/cKingdomModel.hpp"
#include "wii/Mdl0.hpp"

namespace kingdom::game::g04
{

Vector4 ResNodeTrans(kingdom::cKingdomModel* m, const char* node)
{
	const wii::Mdl0* mdl = m != nullptr ? m->pMdl0() : nullptr;
	if (mdl != nullptr)
		for (const wii::Mdl0Node& n : mdl->nodes)
			if (n.name == node) return Vector4(n.translate[0], n.translate[1], n.translate[2], 1.0f);
	return Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

}
