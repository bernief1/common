// =====================
// common/stringutil.cpp
// =====================

#include "fileutil.h"
#include "stringutil.h"

int SkipLeadingWhitespace(const char*& str, const char* end, const char* whitespace)
{
	int skipped = 0;
	while ((end == nullptr || str < end) && *str && strchr(whitespace, *str)) {
		str++;
		skipped++;
	}
	return skipped;
}

int SkipLeadingWhitespace(char*& str, const char* end, const char* whitespace)
{
	return SkipLeadingWhitespace((const char*&)str, end, whitespace);
}

int SkipTrailingWhitespace(char* str, const char* whitespace)
{
	int skipped = 0;
	for (char* end = str + strlen(str); end > str; end--) {
		if (strchr(whitespace, end[-1]) == nullptr) {
			*end = '\0';
			return skipped;
		} else
			skipped++;
	}
	return 0;
}

NameValuePair::NameValuePair(const char* str, const char* end, const char* whitespace)
{
	SkipLeadingWhitespace(str, end, whitespace);
	char temp[1024];
	if (end) {
		strncpy(temp, str, end - str);
		temp[end - str] = '\0'; // null-terminate because strncpy doesn't(!)
	} else
		strcpy(temp, str);
	char* rhs = strchr(temp, '=');
	if (rhs) {
		*rhs++ = '\0';
		SkipLeadingWhitespace(rhs);
		SkipTrailingWhitespace(rhs);
		m_value = rhs;
		SkipTrailingWhitespace(temp);
		m_name = temp;
	} else {
		m_name = "";
		SkipTrailingWhitespace(temp);
		m_value = temp;
	}
	//printf("NameValuePair(\"%s\"): name=\"%s\", value=\"%s\"\n", str, m_name.c_str(), m_value.c_str());
}

NameValuePairs::NameValuePairs(const char* str, char separator, const char* whitespace)
{
	char* temp = new char[strlen(str) + 1];
	strcpy(temp, str);
	char* s = temp;
	while (true) {
		char* end = NextSeparator(s, separator);
		push_back(NameValuePair(s, end, whitespace));
		if (end)
			s = end + 1;
		else
			break;
	}
	delete[] temp;
}

const char* NameValuePairs::NextSeparator(const char* str, char separator)
{
#if 1
	return strchr(str, separator);
#else // TODO -- this needs to be thoroughly tested!
	struct Braces {
		char m_open;
		char m_close;
		const char* m_skip;
		int m_index;
		int m_depth[32];
		int GetDepth() const { return m_depth[m_index]; }
		bool Push(int& depth) { if (m_index < icountof(m_depth)) { m_depth[m_index++] = depth++; return true; } else return false; }
		bool Pop(int& depth) { return m_index > 0 && m_depth[--m_index] == --depth; }
	};
	Braces braces[] = {
		{'\"', '\"', "\\\"", 0},
		{'(', ')', nullptr, 0},
		{'[', ']', nullptr, 0},
		{'{', '}', nullptr, 0},
	};
	int depth = 0;
	for (const char* s = str; *s; s++) {
		for (int i = 0; i < icountof(braces); i++) {
			if (braces[i].GetDepth() > 0) {
				const char* skip = braces[i].m_skip;
				if (skip && strncmp(s, skip, strlen(skip)) == 0) {
					s += strlen(skip) - 1; // skip internal quotes
					continue;
				} else if (*s == braces[i].m_close && braces[0].GetDepth() == 0) {
					if (!braces[i].Pop(depth))
						return nullptr; // syntax error!
				}
			}
			if (*s == braces[i].m_open && braces[0].GetDepth() == 0)
				if (!braces[i].Push(depth))
					return nullptr; // syntax error!
		}
		if (*s == separator && depth == 0)
			return s;
	}
	return nullptr;
#endif
}

char* NameValuePairs::NextSeparator(char* str, char separator)
{
	return (char*)NextSeparator((const char*)str, separator);
}

bool NameValuePairs::HasValue(const char* name, uint32 minIndex, uint32 maxIndex) const
{
	return GetStringValue(name, nullptr, minIndex, maxIndex) != nullptr;
}

const char* NameValuePairs::GetStringValue(const char* name, const char* defaultValue, uint32 minIndex, uint32 maxIndex) const
{
	if (maxIndex == 0)
		maxIndex = (uint32)size() - 1;
	for (uint32 i = minIndex; i <= maxIndex; i++) {
		const NameValuePair& pair = operator[](i);
		if (stricmp(name, pair.m_name.c_str()) == 0)
			return pair.m_value.c_str();
	}
	return defaultValue;
}

float NameValuePairs::GetFloatValue(const char* name, float defaultValue, uint32 minIndex, uint32 maxIndex) const
{
	const char* valueStr = GetStringValue(name, nullptr, minIndex, maxIndex);
	if (valueStr)
		return (float)atof(valueStr);
	else
		return defaultValue;
}

int NameValuePairs::GetIntValue(const char* name, int defaultValue, uint32 minIndex, uint32 maxIndex) const
{
	const char* valueStr = GetStringValue(name, nullptr, minIndex, maxIndex);
	if (valueStr)
		return (int)atoi(valueStr);
	else
		return defaultValue;
}

uint32 NameValuePairs::GetUIntValue(const char* name, uint32 defaultValue, uint32 minIndex, uint32 maxIndex) const
{
	const char* valueStr = GetStringValue(name, nullptr, minIndex, maxIndex);
	if (valueStr)
		return (uint32)atoi(valueStr);
	else
		return defaultValue;
}

bool NameValuePairs::GetBoolValue(const char* name, bool defaultValue, uint32 minIndex, uint32 maxIndex) const
{
	const char* valueStr = GetStringValue(name, nullptr, minIndex, maxIndex);
	if (valueStr)
		return stricmp(valueStr, "TRUE") == 0;
	else
		return defaultValue;
}

PathExt::PathExt(const char* path, const char* ext, ...)
{
	GET_STR_VARARGS(temp,512,ext);
	strcpy(m_path, path);
	char* end = strrchr(m_path, '.');
	if (end)
		strcpy(end, temp);
	else
		strcat(m_path, temp);
}

FindShortenedPathWithExt::FindShortenedPathWithExt(const char* path, const char* ext, int depth)
{
	char path2[512];
	strncpy(path2, path, sizeof(path2));
	char* start = Max(path2, strrchr(path2, '\\'));
	char* ext2 = strrchr(path2, '.');
	if (ext2 && ext2 > start)
		strcpy(ext2, ext);
	else
		strcat(path2, ext);
	while (true) {
		if (FileExists(path2)) {
			strcpy(m_path, path2);
			return;
		} else {
			char* underscore = strrchr(path2, '_');
			if (underscore > start)
				strcpy(underscore, ext);
			else
				break;
		}
	}
	if (depth > 0 && start > path2) {
		*start = '\0';
		strcpy(m_path, FindShortenedPathWithExt(path2, ext, depth - 1));
	} else
		strcpy(m_path, "");
}

void PathInsertDirectory(char* dst, const char* dir, const char* path, bool createDirectory)
{
	// e.g.: PathInsertDirectory(dst, "\\mydir", "a\\b\\c", ..) -> dst="a\\b\\mydir\\c"
	strcpy(dst, path);
	char* lastSlash = strrchr(dst, '\\');
	if (lastSlash) {
		char slashName[512];
		strcpy(slashName, lastSlash);
		strcpy(lastSlash, dir);
		if (createDirectory)
			CreateDirectoryRecursively(dst);
		strcat(dst, slashName);
	} else {
		if (dir[0] == '\\')
			dir++;
		if (createDirectory)
			CreateDirectoryRecursively(dir);
		sprintf("%s\\%s", dir, path);
	}
}

const char* strprintf(char* str, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(str, fmt, args);
	va_end(args);
	return str;
}

const char* strnprintf(char* str, size_t size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(str, size, fmt, args);
	va_end(args);
	return str;
}

const char* strcatf(char* str, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const size_t len = strlen(str);
	vsprintf(str + len, fmt, args);
	va_end(args);
	return str;
}

const char* strncatf(char* str, size_t size, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	const size_t len = strlen(str);
	vsnprintf(str + len, size - len, fmt, args);
	va_end(args);
	return str;
}

//void strcpy_ext(char* dst, const char* src, const char* ext)
//{
//	strcpy(dst, src);
//	char* s = strrchr(dst, '.');
//
//	if (s) { strcpy(s, ext); }
//	else   { strcat(s, ext); }
//}

#if !XXX_GAME
const char* stristr(const char* str, const char* substr, const char* nomatch)
{
	const int len = (int)strlen(substr);
	for (const char* s = str; *s; s++)
		if (strnicmp(s, substr, len) == 0)
			return s;
	return nomatch;
}

char* stristr(char* str, const char* substr, char* nomatch)
{
	const int len = (int)strlen(substr);
	for (char* s = str; *s; s++)
		if (strnicmp(s, substr, len) == 0)
			return s;
	return nomatch;
}
#endif // !XXX_GAME

const char* strskip(const char* str, const char* skip)
{
	const char* s = str;
	while (*s && *s == *skip) { s++; skip++; }
	return (*skip) ? str : s;
}

char* strskip(char* str, const char* skip)
{
	char* s = str;
	while (*s && *s == *skip) { s++; skip++; }
	return (*skip) ? str : s;
}

const char* striskip(const char* str, const char* skip)
{
	const char* s = str;
	while (*s && tolower(*s) == tolower(*skip)) { s++; skip++; }
	return (*skip) ? str : s;
}

char* striskip(char* str, const char* skip)
{
	char* s = str;
	while (*s && tolower(*s) == tolower(*skip)) { s++; skip++; }
	return (*skip) ? str : s;
}

const char* strsearch(const char* str, const char* search)
{
	const char* s = strstr(str, search);
	return s ? strskip(s, search) : nullptr;
}

char* strsearch(char* str, const char* search)
{
	char* s = strstr(str, search);
	return s ? strskip(s, search) : nullptr;
}

bool if_strskip(const char*& str, const char* skip)
{
	const char* s = strskip(str, skip);
	if (s != str) {
		str = s;
		return true;
	} else
		return false;
}

bool if_strskip(char*& str, const char* skip)
{
	char* s = strskip(str, skip);
	if (s != str) {
		str = s;
		return true;
	} else
		return false;
}

bool if_striskip(const char*& str, const char* skip)
{
	const char* s = striskip(str, skip);
	if (s != str) {
		str = s;
		return true;
	} else
		return false;
}

bool if_striskip(char*& str, const char* skip)
{
	char* s = striskip(str, skip);
	if (s != str) {
		str = s;
		return true;
	} else
		return false;
}

bool strstartswith(const char* str, const char* start)
{
	const size_t len = strlen(start);
	for (size_t i = 0; i < len; i++) {
		if (str[i] != start[i])
			return false;
	}
	return true;
}

int chrreplace(char* str, char toreplace, char replacewith)
{
	int count = 0;
	for (char* s = str; *s; s++) {
		if (*s == toreplace) {
			*s = replacewith;
			count++;
		}
	}
	return count;
}