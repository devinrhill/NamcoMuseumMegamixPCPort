#include "ResUserDataItem.hpp"

#include <cmath>
#include <cstdlib>

ResUserDataItem::ResUserDataItem()
{
}

ResUserDataItem::ResUserDataItem(RMDLUserDataParam::UserData* param)
{
	This = param;
}

ResUserDataItem::ResUserDataItem(const wii::Mdl0UserItem* item) : Item(item)
{
}

bool ResUserDataItem::IsValid()
{
	return Item != nullptr;
}

string ResUserDataItem::GetName()
{
	return Item != nullptr ? Item->name : string();
}

// the first value (the converted game code only ever reads single values). String items (type 2) are parsed: the hub's BG nodes
// store their clip speeds as text ("1"), read as 0 before, which froze every hub BG animation (27th session; UNVERIFIED how the DOL reads them)
float ResUserDataItem::GetF32()
{
	if (Item == nullptr) return 0.0f;
	if (!Item->floats.empty()) return Item->floats[0];
	if (!Item->ints.empty()) return (float)Item->ints[0];
	if (Item->type == 2 && !Item->str.empty()) return std::strtof(Item->str.c_str(), nullptr);
	return 0.0f;
}

int ResUserDataItem::GetS32()
{
	if (Item == nullptr) return 0;
	if (!Item->ints.empty()) return Item->ints[0];
	if (!Item->floats.empty()) return (int)std::lround(Item->floats[0]);
	if (Item->type == 2 && !Item->str.empty()) return (int)std::strtol(Item->str.c_str(), nullptr, 0);
	return 0;
}
