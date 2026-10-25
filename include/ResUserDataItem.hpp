#pragma once

#include "common.hpp"
#include "RMDLUserDataParam.hpp"
#include "wii/Mdl0.hpp"

class ResUserDataItem
{
public:
	RMDLUserDataParam::UserData* This{};
	const wii::Mdl0UserItem* Item{};   // native
	explicit ResUserDataItem(const wii::Mdl0UserItem* item);

	ResUserDataItem();

	ResUserDataItem(RMDLUserDataParam::UserData* param);

	bool IsValid();

	string GetName();

	float GetF32();

	int GetS32();
};
