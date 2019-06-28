// ===================
// common/stringutil.h
// ===================

#ifndef _INCLUDE_COMMON_STRINGUTIL_H_
#define _INCLUDE_COMMON_STRINGUTIL_H_

#include "common/common.h"

int SkipLeadingWhitespace(const char*& str, const char* end = nullptr, const char* whitespace = " \t");
int SkipLeadingWhitespace(char*& str, const char* end = nullptr, const char* whitespace = " \t");
int SkipTrailingWhitespace(char* str, const char* whitespace = " \t");

class NameValuePair
{
public:
	NameValuePair(const char* str, const char* end = nullptr, const char* whitespace = " \t");
	
	std::string m_name;
	std::string m_value;
};

class NameValuePairs : public std::vector<NameValuePair>
{
public:
	NameValuePairs(const char* str, char separator = ',', const char* whitespace = " \t");

	static const char* NextSeparator(const char* str, char separator = ',');
	static char* NextSeparator(char* str, char separator = ',');
	bool HasValue(const char* name, uint32 minIndex = 0, uint32 maxIndex = 0) const;
	const char* GetStringValue(const char* name, const char* defaultValue = nullptr, uint32 minIndex = 0, uint32 maxIndex = 0) const;
	float GetFloatValue(const char* name, float defaultValue = 0.0f, uint32 minIndex = 0, uint32 maxIndex = 0) const;
	int GetIntValue(const char* name, int defaultValue = 0, uint32 minIndex = 0, uint32 maxIndex = 0) const;
	uint32 GetUIntValue(const char* name, uint32 defaultValue = 0, uint32 minIndex = 0, uint32 maxIndex = 0) const;
	bool GetBoolValue(const char* name, bool defaultValue = false, uint32 minIndex = 0, uint32 maxIndex = 0) const;
};

class PathExt
{
public:
	PathExt(const char* path, const char* ext, ...);
	const char* c_str() const { return m_path; }
	operator const char*() const { return m_path; }
	char m_path[512];
};

class FindShortenedPathWithExt
{
public:
	FindShortenedPathWithExt(const char* path, const char* ext, int depth = 2);
	const char* c_str() const { return m_path; }
	operator const char*() const { return m_path; }
	operator bool() const { return m_path[0] != '\0'; }
	char m_path[512];
};

void PathInsertDirectory(char* dst, const char* dir, const char* path, bool createDirectory = true);

class varString : public std::string
{
public:
	inline varString(const char* format, ...)
	{
		GET_STR_VARARGS(temp,8192,format);
		std::string::operator=(temp);
	}

	inline operator const char*() const
	{
		return std::string::c_str();
	}
};

template <class T, size_t N> inline void safe_strcpy(T (&out)[N], const T *src)
{
	memcpy(out, src, N*sizeof(T));
	out[N - 1] = static_cast<T>(0);
}

const char* strprintf(char* str, const char* fmt, ...);
const char* strnprintf(char* str, size_t size, const char* fmt, ...);
const char* strcatf(char* str, const char* fmt, ...);
const char* strncatf(char* str, size_t size, const char* fmt, ...);

// override ASCII string functions for wchar_t ..
inline size_t strlen(const wchar_t* str) { return wcslen(str); }
inline wchar_t* strcpy(wchar_t* str, const wchar_t* src) { return wcscpy(str, src); }
inline wchar_t* strcat(wchar_t* str, const wchar_t* src) { return wcscat(str, src); }
inline wchar_t* strchr(wchar_t* str, int ch) { return wcschr(str, (wchar_t)ch); }
inline wchar_t* strrchr(wchar_t* str, int ch) { return wcsrchr(str, (wchar_t)ch); }
//inline int vsprintf(wchar_t* buffer, const wchar_t* format, va_list argptr) { return vswprintf(buffer, format, argptr); }
//inline int vsnprintf(wchar_t* buffer, size_t count, const wchar_t* format, va_list argptr) { return _vsnwprintf(buffer, count, format, argptr); }

template <typename T, size_t size> inline const T* strpathext(T (&buffer)[size], const T* path, const T* fmt, ...)
{
	strcpy(buffer, path);
	T* ext = strrchr(buffer, '.');
	if (ext == nullptr)
		ext = buffer + strlen(buffer);
	va_list args;
	va_start(args, fmt);
	vsnprintf(ext, size - static_cast<ptrdiff_t>(ext - buffer), fmt, args);
	va_end(args);
	return buffer;
}

template <typename T> inline const T* strpathext(T* buffer, size_t size, const T* path, const T* fmt, ...)
{
	strcpy(buffer, path);
	T* ext = strrchr(buffer, '.');
	if (ext == nullptr)
		ext = buffer + strlen(buffer);
	va_list args;
	va_start(args, fmt);
	vsnprintf(ext, size - static_cast<ptrdiff_t>(ext - buffer), fmt, args);
	va_end(args);
	return buffer;
}

//void strcpy_ext(char* dst, const char* src, const char* ext);
#if XXX_GAME
#include "common/DString.h"
#else
const char* stristr(const char* str, const char* substr, const char* nomatch = nullptr);
char* stristr(char* str, const char* substr, char* nomatch = nullptr);
#endif
const char* strskip(const char* str, const char* skip);
char* strskip(char* str, const char* skip);
const char* striskip(const char* str, const char* skip);
char* striskip(char* str, const char* skip);
const char* strsearch(const char* str, const char* search);
char* strsearch(char* str, const char* search);
bool if_strskip(const char*& str, const char* skip);
bool if_strskip(char*& str, const char* skip);
bool if_striskip(const char*& str, const char* skip);
bool if_striskip(char*& str, const char* skip);
bool strstartswith(const char* str, const char* start);
int chrreplace(char* str, char toreplace, char replacewith);

#endif // _INCLUDE_COMMON_STRINGUTIL_H_