#pragma once

#include "common.hpp"

class ResourceCache
{
public:
	enum EType
	{
		Permanent = 0,
		Temporary = 1,
		Max = 2
	};

	class Data
	{
	public:
		std::vector<byte> data{};

		int refCount{};
	};

private:
	static std::vector<std::unordered_map<string, Object*>> m_dData;

public:
	static void Init();

	static void Free();

	static void Unload(EType type);

	static bool Unload(string path);
};
