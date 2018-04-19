#pragma once
#include <map>
#include <string>
#include <algorithm>
#include <atlstr.h>
#include <shlwapi.h>
#include <assert.h>
#ifdef FILE_ARCHIVER_STATIC
#  define FILE_ARCHIVER_API
#else
#  ifdef FILE_ARCHIVER_EXPORTS
#    define FILE_ARCHIVER_API __declspec(dllexport)
#  else
#    define FILE_ARCHIVER_API __declspec(dllimport)
#  endif
#endif
using namespace std;
wstring getFileExtension(wstring filePath);
wstring changeFileExtension(wstring filePath,wstring ext);
wstring standardizePathSeparator(wstring path);
wstring standardizeDirectoryPath(wstring folderPath);
wstring getDirectoryPath(wstring filePath);
wstring getFileName(wstring filePath);
bool isSameFileExtension(wstring ext1,wstring ext2);
#define READ_MODE (STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE)
#define WRITE_MODE (STGM_DIRECT|STGM_WRITE|STGM_SHARE_EXCLUSIVE)
#define FILE_NAME_LENGTH 512
class FileArchiver
{
private:
	FileArchiver(void);
	~FileArchiver(void);
	static void importFile(wstring sourceFilePath,IStorage*pStg);
	static void importDirectory(wstring sourceDirectoryPath,IStorage*pStg);
	static void exportStream(IStorage*pStg,wstring streamName,wstring destinationDirectoryPath);
	static void exportStorage(IStorage*pStg,wstring destinationDirectoryPath);
	static map<wstring,wstring>m_guidNameMap;
	static void loadMap(IStorage*pStg);
	static void saveMap(IStorage*pStg);
public:
	FILE_ARCHIVER_API static void Zip(wstring sourceDirectoryPath,wstring destinationFilePath);
	FILE_ARCHIVER_API static void Unzip(wstring sourceFilePath,wstring destinationDirectoryPath);
};
extern"C"
FILE_ARCHIVER_API void FileArchiver_Zip(wchar_t*sourceDirectoryPath,wchar_t*destinationFilePath);
extern"C"
FILE_ARCHIVER_API void FileArchiver_Unzip(wchar_t*sourceFilePath,wchar_t*destinationDirectoryPath);
