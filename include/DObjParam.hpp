#pragma once

#include "common.hpp"

// [Serializable]
class DObjParam : public ScriptableObject
{
public:
	class sObj;

	// [Serializable]
	class sObjParam
	{
	public:
		uint m_objNum{};

		std::vector<sObj*> m_sObj;

		sObj* GetParam(int x, int y);
	};

	// [Serializable]
	class sObj
	{
	public:
		uint m_type{};

		int m_pos[2]{};

		int m_param[8]{};
	};

	enum EAssign
	{
		Type = 0,
		Pos1 = 1,
		Pos2 = 2,
		Param1 = 3,
		Param2 = 4,
		Param3 = 5,
		Param4 = 6,
		Param5 = 7,
		Param6 = 8,
		Param7 = 9,
		Param8 = 10
	};

	uint m_layerNum{};

	std::vector<sObjParam*> m_sObjParam;
};
