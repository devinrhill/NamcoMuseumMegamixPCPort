#pragma once

#include "common.hpp"

// [Serializable]
class RMDLUserDataParam : public ScriptableObject
{
public:
	class UserData;

	// [Serializable]
	class Param
	{
	public:
		string m_nodeName{};

		int m_nodeIdx{};

		int m_parentIdx{};

		std::vector<Param*> m_childParam;

		std::vector<UserData*> m_userDatas;

		UserData* GetUserData(string dataName);

		UserData* GetUserData(int dataIdx);

		Param* GetParam(string nodeName);

		Param* GetParam(int nodeIdx);

		int GetChildUserDataNum();

		UserData* GetChildUserData(int dataIdx);

	private:
		void GetChildUserData(std::vector<UserData*>& allChildren);
	};

	// [Serializable]
	class UserData
	{
	public:
		string m_dataName{};

		int m_dataIdx{};

		float m_dataParam{};
	};

	std::vector<Param*> m_paramList;

	Param* GetParam(string nodeName);

	Param* GetParam(int nodeIdx);
};
