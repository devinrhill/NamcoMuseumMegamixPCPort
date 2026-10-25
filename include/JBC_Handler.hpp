#pragma once

#include "common.hpp"

class JBC_Element;

class JBC_Handler
{
public:
	bool force{};

	uint new_no{};

	uint cur_no{};

	uint old_no{};

	uint mask{};

	uint shift{};

	uint n_table{};

	JBC_Element* table{};
};
