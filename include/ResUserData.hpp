#pragma once

#include "common.hpp"
#include "RMDLUserDataParam.hpp"
#include "ResUserDataItem.hpp"
#include "wii/Mdl0.hpp"

class ResUserData
{
public:
	RMDLUserDataParam::Param* This{};
	const std::vector<wii::Mdl0UserItem>* Items{};   // native: the node's MDL0 user data (null = none)
	explicit ResUserData(const std::vector<wii::Mdl0UserItem>* items);

	ResUserData();

	ResUserData(RMDLUserDataParam::Param* param);

	bool IsValid();

	int GetItemNumEntries();

	ResUserDataItem* GetItem(int idx);

	ResUserDataItem* GetItem(string name);
};
