// ===================
// common/fileutil.cpp
// ===================

#include "fileutil.h"
#include "stringutil.h"

void StartupMain(int argc, const char* argv[])
{
#if PLATFORM_PC
	if (argc > 0 && !IsDebuggerPresent()) {
		char dir[512];
		strcpy(dir, argv[0]);
		char* slash = strrchr(dir, '\\');
		if (slash) {
			slash[0] = '\0';
			SetCurrentDirectoryA(dir);
		}
	}
#endif // PLATFORM_PC
}

#if PLATFORM_PC
uint32 Params::Load(int argc, const char* argv[], const char* inputPathExtensionList, const char* paramsFileName, const char* paramsFileExt, bool showParams, bool verbose)
{
	if (inputPathExtensionList) {
		m_inputPathExtensionList = inputPathExtensionList;
		for (int i = 1; i < argc; i++) {
			const char* path = argv[i];
			if (path[0] != '#' && path[0] != '-' && CheckInputPathAgainstExtensionList(path)) {
				char temp[512];
				strcpy(temp, path);
				for (char* s = temp; *s; s++) {
					if (*s == '/')
						*s = '\\';
				}
				if (verbose)
					printf("adding input path \"%s\" from \"<args>\" ..\n", temp);
				m_inputPaths.push_back(temp);
			}
		}
		if (verbose)
			printf("added %u input file paths\n", (uint32)m_inputPaths.size());
	}
	uint32 count = 0;
	const char* exePath = argv[0];
	count += LoadFromTextFile_SameDirectoryAsExecutable(exePath, paramsFileName, verbose);
	count += LoadFromTextFile_SameNameAsExecutable(exePath, paramsFileExt, verbose);
	count += LoadFromTextFile_SameDirectoryAsInputPaths(paramsFileName, verbose);
	count += LoadFromTextFile_AssociatedNameWithInputPaths(paramsFileName, verbose);
	count += LoadFromTextFile_SpecifiedInArgs(argc, argv, verbose);
	if (verbose)
		printf("applying params from commandline ..\n");
	for (int i = 1; i < argc; i++) {
		if (LoadFromString("<args>", argv[i], false, verbose))
			count++;
	}
	if (verbose) {
		printf("input paths:\n");
		if (m_inputPaths.empty())
			printf("\t<none>\n");
		else {
			for (uint32 i = 0; i < m_inputPaths.size(); i++)
				printf("\t%04u: %s\n", i, m_inputPaths[i].c_str());
		}
		printf("final param values:\n");
		for (uint32 i = 0; i < m_params.size(); i++) {
			const Param& param = m_params[i];
			if (param.m_from.empty())
				printf("param '%s' was not set (default value %s)\n", param.GetName().c_str(), param.GetValueAsString().c_str());
			else
				printf("param '%s' was set to %s from \"%s\"\n", param.GetName().c_str(), param.GetValueAsString().c_str(), param.m_from.c_str());
		}
		printf("total params parsed = %u\n", count);
	}
	if (showParams) {
		printf("############################\n");
		for (uint32 i = 0; i < m_params.size(); i++) {
			const Param& param = m_params[i];
			printf("%s%s=%s\n", param.m_from.empty() ? "#" : "-", param.GetName().c_str(), param.GetValueAsString().c_str());
		}
		printf("############################\n");
	}
	return count;
}

std::string Params::Param::GetName() const
{
	char temp[256];
	ForceAssert(m_name.front() == '-');
	ForceAssert(m_name.back() == '=');
	strcpy(temp, m_name.c_str() + 1);
	temp[strlen(temp) - 1] = '\0';
	return temp;
}

std::string Params::Param::GetValueAsString() const
{
	char temp[256];
	switch (m_type) {
	case TYPE_FLOAT   : sprintf(temp, "%f", *(const float*)m_ptr); return temp;
	case TYPE_UINT    : sprintf(temp, "%u", *(const uint32*)m_ptr); return temp;
	case TYPE_BOOL    : return *(const bool*)m_ptr ? "true" : "false";
	case TYPE_STRING  : return ((const std::string*)m_ptr)->c_str();
	case TYPE_STRING_P: return (const char*)m_ptr;
	}
	return "";
}

void Params::Param::SetValueFromString(const char* path, const char* value, bool verbose)
{
	if (verbose) {
		printf("param '%s' being %s to %s from \"%s\"", GetName().c_str(), m_from.empty() ? "set" : "overriden", value, path);
		if (m_from.empty())
			printf("\n");
		else
			printf(" (was %s from \"%s\")\n", GetValueAsString().c_str(), m_from.c_str());
	}
	m_from = path;
	switch (m_type) {
	case TYPE_FLOAT   : *(float      *)m_ptr = (float)atof(value); break;
	case TYPE_UINT    : *(uint32     *)m_ptr = (uint32)atoi(value); break;
	case TYPE_BOOL    : *(bool       *)m_ptr = (stricmp(value, "true") == 0); break;
	case TYPE_STRING  : *(std::string*)m_ptr = std::string(value); break;
	case TYPE_STRING_P: strcpy((char*)m_ptr, value); break;
	}
}

bool Params::CheckInputPathAgainstExtensionList(const char* path) const
{
	if (!m_inputPathExtensionList.empty()) {
		const char* ext = strrchr(path, '.');
		if (ext) {
			char ext2[32];
			sprintf(ext2, "*%s;", ext); // inputPathExtensionList e.g. "*.dds;*.png;"
			return stristr(m_inputPathExtensionList.c_str(), ext2) != nullptr;
		}
	}
	return false;
}

bool Params::LoadFromString(const char* path, const char* str, bool addInputPaths, bool verbose)
{
	if (str[0] == '#') {
		// do nothing
	} else if (str[0] == '-') {
		char temp[256];
		if (strrchr(str, '=') == NULL) {
			sprintf(temp, "%s=true", str);
			str = temp;
		}
		bool found = false;
		for (uint32 paramIndex = 0; paramIndex < m_params.size(); paramIndex++) {
			Param& param = m_params[paramIndex];
			if (if_striskip(str, param.m_name.c_str())) {
				param.SetValueFromString(path, str, verbose);
				return true;
			}
		}
		fprintf(stderr, "WARNING: unknown command option '%s' being ignored!\n", str);
	} else if (addInputPaths && CheckInputPathAgainstExtensionList(str)) {
		bool dup = false;
		for (uint32 i = 0; i < m_inputPaths.size(); i++) {
			if (stricmp(m_inputPaths[i].c_str(), str) == 0) {
				dup = true;
				break;
			}
		}
		if (dup)
			fprintf(stderr, "ignoring input path \"%s\" from \"%s\" (duplicate)\n", str, path);
		else {
			if (verbose)
				printf("adding input path \"%s\" from \"%s\" ..\n", str, path);
			m_inputPaths.push_back(str);
		}
	}
	return false;
}

uint32 Params::LoadFromTextFile(const char* path, bool verbose)
{
	uint32 count = 0;
	if (m_searched.find(path) == m_searched.end()) {
		m_searched[path] = true;
		FILE* file = fopen(path, "r");
		if (file) {
			if (verbose)
				printf("loading params file \"%s\" ..\n", path);
			char line[1024];
			while (fgets(line, sizeof(line), file)) {
				if (line[0] != '#') {
					char* end = strpbrk(line, "\r\n");
					if (end)
						*end = '\0';
					if (LoadFromString(path, line, true, verbose))
						count++;
				}
			}
			fclose(file);
		}
	}
	return count;
}

uint32 Params::LoadFromTextFile_SameDirectoryAsExecutable(const char* exePath, const char* paramsFileName, bool verbose)
{
	uint32 count = 0;
	if (paramsFileName) {
		char path[512];
		strcpy(path, exePath);
		char* slash = strrchr(path, '\\');
		if (slash) {
			if (slash[-1] == '\\') // hack - when i run in the debugger, i'm seeing a double-slash before the exe name
				slash--;
			strcpy(slash + 1, paramsFileName);
			if (FileExists(path)) {
				const uint32 n = LoadFromTextFile(path, verbose);
				if (verbose)
					printf("parsed %u params from file \"%s\" (same directory as executable)\n", n, path);
				count += n;
			} else if (verbose)
				printf("no params file \"%s\" found (same directory as executable)\n", path);
		} else if (verbose)
			printf("XXXXXXXXXXXXXX DO NOT SUBMIT no params file \"%s\" found (same directory as executable)\n", path);
	}
	return count;
}

uint32 Params::LoadFromTextFile_SameDirectoryAsInputPaths(const char* paramsFileName, bool verbose)
{
	uint32 count = 0;
	if (paramsFileName) {
		for (uint32 i = 0; i < m_inputPaths.size(); i++) {
			char path[512];
			strcpy(path, m_inputPaths[0].c_str());
			char* slash = strrchr(path, '\\');
			if (slash)
				strcpy(slash + 1, paramsFileName);
			else
				strcpy(path, paramsFileName);
			if (FileExists(path)) {
				const uint32 n = LoadFromTextFile(path, verbose);
				if (verbose)
					printf("parsed %u params from file \"%s\" (same directory as input path)\n", n, path);
				count += n;
			} else if (verbose)
				printf("no params file \"%s\" found (same directory as input path)\n", path);
		}
	}
	return count;
}

uint32 Params::LoadFromTextFile_SameNameAsExecutable(const char* exePath, const char* paramsFileExt, bool verbose)
{
	uint32 count = 0;
	if (paramsFileExt) {
		char path[512];
		strcpy(path, PathExt(exePath, paramsFileExt));
		if (FileExists(path)) {
			const uint32 n = LoadFromTextFile(path, verbose);
			if (verbose)
				printf("parsed %u params from file \"%s\" (same name as executable)\n", n, path);
			count += n;
		} else if (verbose)
			printf("no params file \"%s\" found (same name as executable)\n", path);
	}
	return count;
}

uint32 Params::LoadFromTextFile_AssociatedNameWithInputPaths(const char* paramsFileExt, bool verbose)
{
	uint32 count = 0;
	if (paramsFileExt) {
		for (uint32 i = 0; i < m_inputPaths.size(); i++) {
			const char* path = m_inputPaths[i].c_str();
			char path2[512];
			strcpy(path2, FindShortenedPathWithExt(path, paramsFileExt));
			if (path2[0]) {
				const uint32 n = LoadFromTextFile(path, verbose);
				if (verbose)
					printf("parsed %u params from file \"%s\" (specific)\n", n, path);
				count += n;
			} else if (verbose)
				printf("no params file associated with input path \"%s\" found\n", path);
		}
	}
	return count;
}

uint32 Params::LoadFromTextFile_SpecifiedInArgs(int argc, const char* argv[], bool verbose)
{
	uint32 count = 0;
	for (int i = 1; i < argc; i++) {
		const char* path = argv[i];
		if (path[0] != '#' && path[0] != '-') {
			const char* ext = strrchr(path, '.');
			if (ext && stricmp(ext, ".txt") == 0) {
				const uint32 n = LoadFromTextFile(path, verbose);
				if (verbose)
					printf("parsed %u params from file \"%s\" (specified in args)\n", n, path);
				count += n;
			}
		}
	}
	return count;
}

// modified from https://gist.github.com/danzek/d7192d250c951804dec05125f5223a30
bool CreateDirectoryRecursively(const char* dir)
{
	const DWORD fileAttributes = GetFileAttributesA(dir);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		const char* slash = strrchr(dir, '\\');
		if (slash) {
			char parent[512];
			strcpy(parent, dir);
			strrchr(parent, '\\')[0] = '\0';
			if (!CreateDirectoryRecursively(parent))
				return false;
		}
		if (!CreateDirectoryA(dir, nullptr)) {
			fprintf(stderr, "CreateDirectory failed for \"%s\"!\n", dir);
			return false;
		}
	} else if ((fileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) == 0) {
		fprintf(stderr, "could not create directory \"%s\" because a file exists with that path!\n", dir);
		return false; // specified directory name already exists as a file or directory
	}
	return true;
}

#define TEMPFILE_PATH "c:/temp_"

void BuildSearchList(std::vector<std::string>& files, const char* path, const char* ext1, const char* ext2, bool preserveCase)
{
	char cmd[1024] = "";
	sprintf(cmd, "dir $S $B \"%s\" > %s", path, TEMPFILE_PATH);
	for (char* s = cmd; *s; s++) {
		if (*s == '/')
			*s = '\\';
		else if (*s == '$') // this is so we can insert forward slashes into the command that don't get changed to backslashes
			*s = '/';
	}
	system(cmd);
	FILE* file = fopen(TEMPFILE_PATH, "r");
	if (file) {
		char line[1024] = "";
		while (rage_fgetline(line, sizeof(line), file)) {
			char* path = line;
			while (*path == ' ' || *path == '\t')
				path++; // skip leading whitespace
			for (char* s = path; *s; s++) {
				if (*s == '\\')
					*s = '/';
			}
			if (!preserveCase)
				_strlwr(path);
			const char* ext = strrchr(path, '.');
			bool skip = false;
			if (ext && (ext1 || ext2)) {
				skip = true;
				if (ext1 && stricmp(ext, ext1) == 0)
					skip = false;
				if (ext2 && stricmp(ext, ext2) == 0)
					skip = false;
			}
			if (!skip)
				files.push_back(path);
		}
		fclose(file);
	}
}
#endif // PLATFORM_PC

bool FileExists(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file) {
		fclose(file);
		return true;
	}
	return false;
}

bool FileExistsAndIsNotZeroBytes(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		const long size = ftell(file);
		fclose(file);
		return size > 0;
	}
	return false;
}

size_t rage_fgetline(char* line, size_t size, FILE* file)
{
	size_t stored;
	do {
		if (fgets(line, (int)size, file))
			stored = strlen(line);
		else
			stored = 0;
		if (stored == 0)
			break; // real EOF
		while (stored && line[stored - 1] <= ' ') // note that \r, \n, etc. are all < ' '
			--stored;
		line[stored] = '\0';
	} while (stored == 0);
	return stored;
}

// pass ext = NULL to always convert '.' to 'p' (e.g. floating point strings)
// pass ext = path + strlen(path) to never convert '.' to 'p'
// otherwise pass ext = strrchr(path, '.')
std::string MakePathCompatible(const char* path, const char* ext)
{
	char temp[512];
	char* dst = temp;
	while (true) {
		const char ch = *path;
		if (ch == '\0')
			break;
		else if (strchr("\\/:*?\"<>|", ch))
			*dst++ = '_';
		else if (ch == '.' && (ext == NULL || path < ext))
			*dst++ = 'p'; // change periods to 'p' so as not to confuse with extensions
		else
			*dst++ = ch;
		path++;
	}
	*dst = '\0';
	return temp;
}

std::string MakePathCompatibleStringf(const char* format, ...)
{
	GET_STR_VARARGS(temp,512,format);
	return MakePathCompatible(temp, NULL);
}