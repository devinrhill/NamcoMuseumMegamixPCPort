#pragma once

#include "common.hpp"

class CommonUtil
{
public:
	template<class T>
	static T* AddMissingComponent(GameObject* go)
	{
		T* val = nullptr; // TODO(unity): val = go.GetComponent<T>()
		if (val == nullptr)
		{
			// TODO(unity): val = go.AddComponent<T>();
		}
		return val;
	}

	static void RegisterUndo(Object* obj, string name);

	static void SetDirty(Object* obj);

	static string GetClockTimeString(int hour, int minute);

	static std::vector<GameObject*> GetAll(GameObject* obj, bool self = false);

	static void GetChildren(GameObject* obj, std::vector<GameObject*>& allChildren);

	static GameObject* GetObject(GameObject* obj, string objName);

	static std::vector<GameObject*> GetObjects(GameObject* obj, string objName);

	static Color ConvertRGB(uint val);

	static void SetLayer(GameObject* obj, string layer);

	static void SetLayer(GameObject* obj, int layer);

private:
	struct DateTime { int Hour; int Minute; };       // only what GetGameTimeFromString needs
	static DateTime GetDateTimeFromString(string str);

	static float GetGameTimeFromString(string str);

public:
	// C# Enum.Parse has no C++ equivalent: provide a specialisation per enum, e.g.
	//   template<> bool CommonUtil::TryParse<EFoo>(string value, bool ignoreCase, EFoo& result)
	// The generic version reports failure, which is what the C# does for a non-enum T.
	template<class T>
	static bool TryParse(string value, bool ignoreCase, T& result)
	{
		result = T();
		return false;
	}

	static string ConvertTimeToHHHMMSS(int second);

	static string ConvertTimeMMSSFF(int msec);

	static string RemoveTag(string text);

	static IEnumerator* UnusedAssets();
};
