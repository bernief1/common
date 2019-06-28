// =================
// common/fileutil.h
// =================

#ifndef _INCLUDE_COMMON_FILEUTIL_H_
#define _INCLUDE_COMMON_FILEUTIL_H_

#include "common/common.h"

void StartupMain(int argc, const char* argv[]);

#if PLATFORM_PC
class Params
{
public:
	template <typename T> inline void Add(const char* name, T& var)
	{
		ForceAssert(name[0] == '-');
		ForceAssert(name[strlen(name) - 1] == '=');
		m_params.push_back(Param(name, &var));
	}

	uint32 Load(int argc, const char* argv[], const char* inputPathExtensionList, const char* paramsFileName = "params.txt", const char* paramsFileExt = ".params", bool showParams = true, bool verbose = false);
	std::vector<std::string>& GetInputPaths() { return m_inputPaths; }

private:
	class Param
	{
	public:
		enum Type
		{
			TYPE_FLOAT   ,
			TYPE_UINT    ,
			TYPE_BOOL    ,
			TYPE_STRING  ,
			TYPE_STRING_P,
			TYPE_COUNT
		};
		Param(const char* name, float      * ptr) : m_name(name), m_type(TYPE_FLOAT   ), m_ptr(ptr) {}
		Param(const char* name, uint32     * ptr) : m_name(name), m_type(TYPE_UINT    ), m_ptr(ptr) {}
		Param(const char* name, bool       * ptr) : m_name(name), m_type(TYPE_BOOL    ), m_ptr(ptr) {}
		Param(const char* name, std::string* ptr) : m_name(name), m_type(TYPE_STRING  ), m_ptr(ptr) {}
		Param(const char* name, char       * ptr) : m_name(name), m_type(TYPE_STRING_P), m_ptr(ptr) {}

		std::string GetName() const;
		std::string GetValueAsString() const;
		void SetValueFromString(const char* path, const char* value, bool verbose);

		std::string m_name;
		std::string m_from;
		Type m_type;
		void* m_ptr;
	};

	bool CheckInputPathAgainstExtensionList(const char* path) const;
	bool LoadFromString(const char* path, const char* str, bool addInputPaths, bool verbose);
	uint32 LoadFromTextFile(const char* path, bool verbose);

	uint32 LoadFromTextFile_SameDirectoryAsExecutable(const char* exePath, const char* paramsFileName, bool verbose);
	uint32 LoadFromTextFile_SameDirectoryAsInputPaths(const char* paramsFileName, bool verbose);
	uint32 LoadFromTextFile_SameNameAsExecutable(const char* exePath, const char* paramsFileExt, bool verbose);
	uint32 LoadFromTextFile_AssociatedNameWithInputPaths(const char* paramsFileExt, bool verbose);
	uint32 LoadFromTextFile_SpecifiedInArgs(int argc, const char* argv[], bool verbose);

	std::string m_inputPathExtensionList;
	std::vector<std::string> m_inputPaths;
	std::vector<Param> m_params;
	std::map<std::string,bool> m_searched;
};

bool CreateDirectoryRecursively(const char* dir);
void BuildSearchList(std::vector<std::string>& files, const char* path, const char* ext1 = NULL, const char* ext2 = NULL, bool preserveCase = false);
#endif // PLATFORM_PC

bool FileExists(const char* path);
bool FileExistsAndIsNotZeroBytes(const char* path);

size_t rage_fgetline(char* line, size_t size, FILE* file);

std::string MakePathCompatible(const char* path, const char* ext = NULL);
std::string MakePathCompatibleStringf(const char* format, ...);

#endif // _INCLUDE_COMMON_FILEUTIL_H_