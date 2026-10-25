#include "kingdom/ResourceDatabase.hpp"

#include "CSharpCompat.hpp"
#include "ArcsvParam.hpp"
#include "wii/Archive.hpp"

namespace kingdom
{

std::unordered_map<std::string, std::string> ResourceDatabase::dataBase;

void ResourceDatabase::Set(string path)
{
	ArcsvParam* arcsvParam = nullptr; // TODO(unity): arcsvParam = Resources.Load(path) as ArcsvParam
	if (nullptr == arcsvParam)
	{
		// TODO(unity): Debug.LogWarning("[ResourceDatabase.Set] 指定されたARCSVが見つかりませんでした " + path);
		return;
	}
	// TODO(unity): Debug.Log("<color=cyan>[ResourceDatabase.Set] " + path + " を読み込み</color>");
	dataBase.clear();
	for (ArcsvParam::ArcsvData* item : arcsvParam->list)
	{
		dataBase.emplace(item->key, CSharpCompat::Replace(item->path, "%%", "ja"));
	}
}

void ResourceDatabase::Overwrite(string path)
{
	ArcsvParam* arcsvParam = nullptr; // TODO(unity): arcsvParam = Resources.Load(path) as ArcsvParam
	if (nullptr == arcsvParam)
	{
		// TODO(unity): Debug.LogWarning("[ResourceDatabase.Overwrite] 指定されたARCSVが見つかりませんでした " + path);
		return;
	}
	for (ArcsvParam::ArcsvData* item : arcsvParam->list)
	{
		if (dataBase.contains(item->key))
		{
			dataBase[item->key] = CSharpCompat::Replace(item->path, "%%", "ja");
		}
		else
		{
			dataBase.emplace(item->key, CSharpCompat::Replace(item->path, "%%", "ja"));
		}
	}
}

CSVParam* ResourceDatabase::CsvMarker(const string& key)
{
	static CSVParam marker;
	std::size_t size = 0;
	return wii::ArchiveStore::Find(key, size) != nullptr ? &marker : nullptr;
}

string ResourceDatabase::GetPath(string key)
{
	if (dataBase.contains(key))
	{
		return dataBase[key].c_str();
	}
	return key; // native build: keys are the archive entry names themselves
}

}
