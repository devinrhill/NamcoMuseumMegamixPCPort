#include "CustomKingdomUtil.hpp"

#include "CSharpCompat.hpp"

float CustomKingdomUtil::scFloatEqualTh = 1E-05f;

bool CustomKingdomUtil::Assert(bool result, string message)
{
	return result;
}

bool CustomKingdomUtil::IF(float value)
{
	return !Mathf::Approximately(0.0f, value);
}

bool CustomKingdomUtil::IsEqualFloat(float value1, float value2)
{
	if (value1 > value2)
	{
		return value1 - value2 < scFloatEqualTh;
	}
	return value2 - value1 < scFloatEqualTh;
}

bool CustomKingdomUtil::IF(int value)
{
	return value != 0;
}

bool CustomKingdomUtil::IsNearInt(int value1, int value2, int sub)
{
	if (value1 > value2)
	{
		return value1 - value2 <= sub;
	}
	return value2 - value1 <= sub;
}

bool CustomKingdomUtil::IF(uint value)
{
	return value != 0;
}

bool CustomKingdomUtil::IF(long value)
{
	return value != 0;
}

bool CustomKingdomUtil::IF(ulong value)
{
	return value != 0;
}

bool CustomKingdomUtil::IF(Vector4 value)
{
	return Vector4::zero != value;
}

bool CustomKingdomUtil::IF(string value)
{
	return !value.empty();
}

int CustomKingdomUtil::strcmp(string str1, string str2)
{
	return CSharpCompat::IndexOf(str1, str2, 0);
}

int CustomKingdomUtil::strncmp(string str1, string str2, int count)
{
	return CSharpCompat::IndexOf(str1, str2, 0, count);
}

bool CustomKingdomUtil::strstr(string str1, string str2)
{
	int num = CSharpCompat::IndexOf(str1, str2, 0);
	return 0 <= num;
}

void CustomKingdomUtil::snprintf(string& outStr, int size, string strFmt, string strParam)
{
	strFmt = CSharpCompat::Replace(strFmt, "%s", "{0}");
	outStr = CSharpCompat::Format(strFmt, strParam);
}

int CustomKingdomUtil::strlen(string str)
{
	return (int)str.length();
}

string CustomKingdomUtil::strrchr(string str, char search)
{
	int num = CSharpCompat::LastIndexOf(str, search);
	if (num >= 0)
	{
		return str.substr(num);
	}
	return string();
}

Vector3 CustomKingdomUtil::ConvertEuler(const Vector3& origin)
{
	Quaternion quaternion = Quaternion::AngleAxis(origin.x, Vector3::right);
	Quaternion quaternion2 = Quaternion::AngleAxis(origin.y, Vector3::up);
	return (Quaternion::AngleAxis(origin.z, Vector3::forward) * quaternion2 * quaternion).eulerAngles();
}
