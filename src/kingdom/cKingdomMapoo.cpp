#include "kingdom/cKingdomMapoo.hpp"

#include <cstdio>
#include <cstring>

#include "kingdom/ResourceDatabase.hpp"
#include "wii/Archive.hpp"
#include "platform/devtools_g07edit.hpp"

namespace kingdom
{

// The object placement file (archive entry "MAPOO_OBJ", big endian; the Unity port had it as a ScriptableObject DObjParam):
//   0x00 "MAPO" "OBJ?" u32 (version / date), 0x0c u32 layer count, 0x10 four zero words, 0x20 layer table {u32 object count, u32 byte offset} per layer,
//   objects of 64 bytes: u32 type, i32 pos[2] (panel grid x / y), i32 param[13] (the code reads 0..3). Layout derived from g07w01s01.lzs (3 layers
//   3 / 1 / 111 objects fill the 7416 bytes exactly); no DOL function was traced for it (UNVERIFIED).
bool cKingdomMapoo::RegistObj(string obj)
{
	std::size_t size = 0;
	const std::uint8_t* d = wii::ArchiveStore::Find(obj, size);
	devtools::G07EditOverride(obj, d, size); // devtools: the dev window's Pac-Motos editor swaps in its edited MAPOO_OBJ (not 1:1)
	if (d == nullptr || size < 0x20)
	{
		std::fprintf(stderr, "cKingdomMapoo: %s not found\n", obj.c_str());
		return false;
	}
	auto rd = [&](std::size_t o) { return (std::uint32_t)d[o] << 24 | (std::uint32_t)d[o + 1] << 16 | (std::uint32_t)d[o + 2] << 8 | d[o + 3]; };
	m_Obj = new DObjParam();
	m_Obj->m_layerNum = rd(0x0c);
	for (uint layer = 0; layer < m_Obj->m_layerNum; layer++)
	{
		auto* lp = new DObjParam::sObjParam();
		const std::size_t desc = 0x20 + 8 * (std::size_t)layer;
		lp->m_objNum = desc + 8 <= size ? rd(desc) : 0;
		const std::size_t off = desc + 8 <= size ? rd(desc + 4) : 0;
		for (uint i = 0; i < lp->m_objNum && off + 64 * (i + 1) <= size; i++)
		{
			auto* o = new DObjParam::sObj();
			const std::size_t p = off + 64 * (std::size_t)i;
			o->m_type = rd(p);
			o->m_pos[0] = (int)rd(p + 4);
			o->m_pos[1] = (int)rd(p + 8);
			for (int k = 0; k < 8; k++) o->m_param[k] = (int)rd(p + 12 + 4 * k);
			lp->m_sObj.push_back(o);
		}
		lp->m_objNum = (uint)lp->m_sObj.size();
		m_Obj->m_sObjParam.push_back(lp);
	}
	return true;
}

uint cKingdomMapoo::nObjLayer()
{
	return m_Obj->m_layerNum;
}

uint cKingdomMapoo::nObjNum(uint layer)
{
	return m_Obj->m_sObjParam[(int)layer]->m_objNum;
}

DObjParam::sObj* cKingdomMapoo::pObj(uint layer, uint num)
{
	return (m_Obj->m_sObjParam[(int)layer]->m_sObj)[(int)num];
}

DObjParam::sObj* cKingdomMapoo::pObj(uint layer, int x, int y)
{
	return m_Obj->m_sObjParam[(int)layer]->GetParam(x, y);
}

}
