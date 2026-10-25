#include "RMDLUserDataParam.hpp"

RMDLUserDataParam::UserData* RMDLUserDataParam::Param::GetUserData(string dataName)
{
	auto it = std::find_if(m_userDatas.begin(), m_userDatas.end(), [&](UserData* v) { return v->m_dataName == dataName; });
	return (it != m_userDatas.end()) ? *it : nullptr;
}

RMDLUserDataParam::UserData* RMDLUserDataParam::Param::GetUserData(int dataIdx)
{
	auto it = std::find_if(m_userDatas.begin(), m_userDatas.end(), [&](UserData* v) { return v->m_dataIdx == dataIdx; });
	return (it != m_userDatas.end()) ? *it : nullptr;
}

RMDLUserDataParam::Param* RMDLUserDataParam::Param::GetParam(string nodeName)
{
	if (m_nodeName == nodeName)
	{
		return this;
	}
	for (Param* item : m_childParam)
	{
		Param* param = item->GetParam(nodeName);
		if (param != nullptr)
		{
			return param;
		}
	}
	return nullptr;
}

RMDLUserDataParam::Param* RMDLUserDataParam::Param::GetParam(int nodeIdx)
{
	if (m_nodeIdx == nodeIdx)
	{
		return this;
	}
	for (Param* item : m_childParam)
	{
		Param* param = item->GetParam(nodeIdx);
		if (param != nullptr)
		{
			return param;
		}
	}
	return nullptr;
}

int RMDLUserDataParam::Param::GetChildUserDataNum()
{
	int num = (int)m_userDatas.size();
	for (Param* item : m_childParam)
	{
		num += item->GetChildUserDataNum();
	}
	return num;
}

RMDLUserDataParam::UserData* RMDLUserDataParam::Param::GetChildUserData(int dataIdx)
{
	std::vector<UserData*> allChildren;
	GetChildUserData(allChildren);
	if (dataIdx < (int)allChildren.size())
	{
		return allChildren[dataIdx];
	}
	return nullptr;
}

void RMDLUserDataParam::Param::GetChildUserData(std::vector<UserData*>& allChildren)
{
	allChildren.insert(allChildren.end(), m_userDatas.begin(), m_userDatas.end());
	for (Param* item : m_childParam)
	{
		item->GetChildUserData(allChildren);
	}
}

RMDLUserDataParam::Param* RMDLUserDataParam::GetParam(string nodeName)
{
	for (Param* param2 : m_paramList)
	{
		Param* param = param2->GetParam(nodeName);
		if (param != nullptr)
		{
			return param;
		}
	}
	return nullptr;
}

RMDLUserDataParam::Param* RMDLUserDataParam::GetParam(int nodeIdx)
{
	for (Param* param2 : m_paramList)
	{
		Param* param = param2->GetParam(nodeIdx);
		if (param != nullptr)
		{
			return param;
		}
	}
	return nullptr;
}
