#include "ResUserData.hpp"

#include "ResUserDataItem.hpp"

// Native: the user data of an MDL0 node (see wii::Mdl0UserItem); the Unity version read an RMDLUserDataParam asset.
ResUserData::ResUserData()
{
}

ResUserData::ResUserData(RMDLUserDataParam::Param* param)
{
	This = param;
}

ResUserData::ResUserData(const std::vector<wii::Mdl0UserItem>* items) : Items(items)
{
}

bool ResUserData::IsValid()
{
	return Items != nullptr && !Items->empty();
}

int ResUserData::GetItemNumEntries()
{
	return Items != nullptr ? (int)Items->size() : 0;
}

ResUserDataItem* ResUserData::GetItem(int idx)
{
	if (Items == nullptr || idx < 0 || idx >= (int)Items->size()) return new ResUserDataItem();
	return new ResUserDataItem(&(*Items)[idx]);
}

ResUserDataItem* ResUserData::GetItem(string name)
{
	if (Items != nullptr)
		for (const auto& it : *Items)
			if (it.name == name) return new ResUserDataItem(&it);
	return new ResUserDataItem();
}
