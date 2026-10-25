#pragma once

// Helpers for C# BCL string behaviour that has no single std:: call.
// Everything else maps directly onto std:: (see CONVERSION_NOTES.md).

#include <string>
#include "common.hpp"

#include <charconv>
#include <limits>

namespace CSharpCompat
{

template<class T> inline std::string ToString(const T& v) { return std::to_string(v); }
inline std::string ToString(const std::string& v) { return v; }
// int.ToString("00"): zero padded to the number of zeros in the format
inline std::string ToString(int v, const char* fmt)
{
	std::string s = std::to_string(v);
	const std::size_t w = std::string(fmt).size();
	while (s.size() < w) s.insert(s.begin(), '0');
	return s;
}
template<class V, class T> inline bool Remove(std::vector<V>& list, const T& item)
{
	for (std::size_t i = 0; i < list.size(); i++) if (list[i] == item) { list.erase(list.begin() + i); return true; }
	return false;
}
template<class V, class T> inline int IndexOf(const std::vector<V>& list, const T& item)
{
	for (std::size_t i = 0; i < list.size(); i++) if (list[i] == item) return (int)i;
	return -1;
}


// string.Replace(oldValue, newValue): replaces every occurrence.
inline std::string Replace(const std::string& str, const std::string& oldValue, const std::string& newValue)
{
	if (oldValue.empty())
	{
		return str;
	}
	std::string result = str;
	std::size_t pos = 0;
	while ((pos = result.find(oldValue, pos)) != std::string::npos)
	{
		result.replace(pos, oldValue.length(), newValue);
		pos += newValue.length();
	}
	return result;
}

// string.Trim(char): strips the character from both ends.
inline std::string Trim(const std::string& str, char trimChar)
{
	std::size_t start = str.find_first_not_of(trimChar);
	if (start == std::string::npos)
	{
		return string();
	}
	std::size_t end = str.find_last_not_of(trimChar);
	return str.substr(start, end - start + 1);
}

// string.IndexOf(value, startIndex): -1 when not found.
inline int IndexOf(const std::string& str, const std::string& value, int startIndex = 0)
{
	std::size_t pos = str.find(value, (std::size_t)startIndex);
	return pos == std::string::npos ? -1 : (int)pos;
}

// string.IndexOf(value, startIndex, count): the match must lie inside [startIndex, startIndex + count).
inline int IndexOf(const std::string& str, const std::string& value, int startIndex, int count)
{
	std::size_t pos = str.substr(0, (std::size_t)(startIndex + count)).find(value, (std::size_t)startIndex);
	return pos == std::string::npos ? -1 : (int)pos;
}

// string.IndexOf(char): -1 when not found.
inline int IndexOf(const std::string& str, char value)
{
	std::size_t pos = str.find(value);
	return pos == std::string::npos ? -1 : (int)pos;
}

// string.LastIndexOf(char): -1 when not found.
inline int LastIndexOf(const std::string& str, char value)
{
	std::size_t pos = str.rfind(value);
	return pos == std::string::npos ? -1 : (int)pos;
}

// string.Format(format, arg0) for the single-argument "{0}" case (with "{{" / "}}" escapes).
inline std::string Format(const std::string& format, const std::string& arg0)
{
	std::string result;
	for (std::size_t i = 0; i < format.length(); i++)
	{
		if (format.compare(i, 3, "{0}") == 0)
		{
			result += arg0;
			i += 2;
		}
		else if (format.compare(i, 2, "{{") == 0 || format.compare(i, 2, "}}") == 0)
		{
			result += format[i];
			i += 1;
		}
		else
		{
			result += format[i];
		}
	}
	return result;
}

// Strips the whitespace and leading '+' that .NET NumberStyles.Integer / NumberStyles.Float accept.
inline std::string NumberTrim(const std::string& str)
{
	const char* ws = " \t\n\v\f\r";
	std::size_t start = str.find_first_not_of(ws);
	if (start == std::string::npos)
	{
		return std::string();
	}
	std::size_t end = str.find_last_not_of(ws);
	std::string result = str.substr(start, end - start + 1);
	if (!result.empty() && result[0] == '+')
	{
		result.erase(0, 1);
	}
	return result;
}

// int.TryParse(s, NumberStyles.Integer, en-US, out result)
inline bool TryParseInt(const std::string& str, int& result)
{
	result = 0;
	std::string text = NumberTrim(str);
	const char* first = text.data();
	const char* last = text.data() + text.size();
	std::from_chars_result r = std::from_chars(first, last, result);
	if (r.ec != std::errc() || r.ptr != last || text.empty())
	{
		result = 0;
		return false;
	}
	return true;
}

// float.TryParse(s, NumberStyles.Float, en-US, out result)
inline bool TryParseFloat(const std::string& str, float& result)
{
	result = 0.0f;
	std::string text = NumberTrim(str);
	const char* first = text.data();
	const char* last = text.data() + text.size();
	std::from_chars_result r = std::from_chars(first, last, result);
	if (r.ec != std::errc() || r.ptr != last || text.empty())
	{
		result = 0.0f;
		return false;
	}
	return true;
}

}
