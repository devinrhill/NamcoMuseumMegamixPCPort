#pragma once

#include "common.hpp"

template<class T>
class Ref
{
public:
	T Value{};

	Ref()
	{
	}

	Ref(T value)
	{
		Value = value;
	}
};
