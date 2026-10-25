#pragma once

#include "common.hpp"

class CustomKingdomUtil
{
public:
	static float scFloatEqualTh;

	static bool Assert(bool result, string message = string());

	static bool IF(float value);

	static bool IsEqualFloat(float value1, float value2);

	static bool IF(int value);

	static bool IsNearInt(int value1, int value2, int sub);

	static bool IF(uint value);

	static bool IF(long value);

	static bool IF(ulong value);

	static bool IF(Vector4 value);

	static bool IF(string value);

	static int strcmp(string str1, string str2);

	static int strncmp(string str1, string str2, int count);

	static bool strstr(string str1, string str2);

	static void snprintf(string& outStr, int size, string strFmt, string strParam);

	static int strlen(string str);

	static string strrchr(string str, char search);

	static Vector3 ConvertEuler(const Vector3& origin);
};
