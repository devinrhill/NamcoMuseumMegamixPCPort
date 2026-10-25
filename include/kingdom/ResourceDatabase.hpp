#pragma once

#include "common.hpp"
#include <ResourceCache.hpp>
#include <type_traits>
#include "kingdom/ResHandle.hpp"
#include "CSVParam.hpp"

namespace kingdom
{

class ResourceDatabase
{
private:
	static std::unordered_map<std::string, std::string> dataBase;

public:
	// non-null marker when an attached archive has the entry (stand-in for the Unity CSVParam asset), else nullptr
	static CSVParam* CsvMarker(const string& key);

	static void Set(string path);

	static void Overwrite(string path);

	static string GetPath(string key);

	template<class T>
	static T* Get(string key, ResourceCache::EType type = ResourceCache::EType::Permanent)
	{
		// native build: generic assets are handles to archive entries (see ResHandle.hpp); the Unity ScriptableObject types (CSVParam, ...) are not available
		if constexpr (std::is_same_v<T, Object> || std::is_same_v<T, AnimationClip>)
			return MakeResHandle(GetPath(key));
		else if constexpr (std::is_same_v<T, CSVParam>)
			return CsvMarker(GetPath(key));   // the callers only test for presence, then read the text through cKingdomCSV::RegistData(key)
		else
			return nullptr;
	}

	template<class T>
	static IEnumerator* GetAsync(string key, ResourceCache::EType type = ResourceCache::EType::Permanent, Action<T*> onCompleted = nullptr)
	{
		// TODO(unity): coroutine; original C# body:
		// 	if (dataBase == null)
		// 	{
		// 		Debug.LogWarning("[ResourceDatabase.Get] Databaseが未設定");
		// 		yield return ResourceCache.GetAsync(key, type, onCompleted);
		// 	}
		// 	else if (dataBase.ContainsKey(key))
		// 	{
		// 		yield return ResourceCache.GetAsync(dataBase[key], type, onCompleted);
		// 	}
		// 	else
		// 	{
		// 		yield return ResourceCache.GetAsync(key, type, onCompleted);
		// 	}
		return nullptr;
	}
};

}
