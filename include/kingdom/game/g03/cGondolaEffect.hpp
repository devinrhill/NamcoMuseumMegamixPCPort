#pragma once

#include "common.hpp"
#include "kingdom/cKingdomPtkHnd.hpp"

namespace kingdom::game::g03
{

class cGondolaEffect : public cKingdomPtkHnd
{
public:
	enum EEffType
	{
		EFF_TYPE_NONE = 0,
		EFF_TYPE_STER = 1,
		EFF_TYPE_WIND = 2
	};

private:
	Vector4 m_vTransL{};

	Vector4 m_vRotateL{};

	uint m_nType{};

public:
	Vector4& vTransL();

	float vTransL(int n);

	Vector4& vRotateL();

	float vRotateL(int n);

	uint& nType();

	cGondolaEffect();

	virtual ~cGondolaEffect();

private:
	void Init();

	void Term();
};

}
