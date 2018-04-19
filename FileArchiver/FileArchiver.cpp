#include "FileArchiver.h"
map<wstring,wstring>FileArchiver::m_guidNameMap=map<wstring,wstring>();
FileArchiver::FileArchiver(void)
{
}
FileArchiver::~FileArchiver(void)
{
}
wstring getFileExtension(wstring filePath)
{
	wstring fileExtension=L"";
	wstring::size_type indexOfLastDot=filePath.find_last_of(L'.');
	if(wstring::npos!=indexOfLastDot)
	{
		fileExtension=filePath.substr(indexOfLastDot);
	}
	return fileExtension;
}
wstring changeFileExtension(wstring filePath,wstring newFileExtension)
{
	wstring newFilePath=L"";
	wstring::size_type indexOfLastDot=filePath.find_last_of(L'.');
	if(wstring::npos!=indexOfLastDot)
	{
		newFilePath=filePath.substr(0,indexOfLastDot);
	}
	else
	{
		return filePath;
	}
	newFilePath+=newFileExtension;
	return newFilePath;
}
wstring standardizePathSeparator(wstring path)
{
	replace(path.begin(),path.end(),L'/',L'\\');
	return path;
}
wstring standardizeDirectoryPath(wstring directoryPath)
{
	directoryPath=standardizePathSeparator(directoryPath);
	if(directoryPath[directoryPath.length()-1]!=L'\\')
	{
		directoryPath.append(L"\\");
	}
	return directoryPath;
}
wstring getDirectoryPath(wstring filePath)
{
	wstring filePath_=standardizePathSeparator(filePath);
	wstring::size_type indexOfLastPathSeparator=filePath_.find_last_of(L'\\');
	wstring directoryPath=filePath;
	if(wstring::npos!=indexOfLastPathSeparator)
	{
		directoryPath=filePath.substr(0,indexOfLastPathSeparator+1);
	}
	return directoryPath;
}
wstring getFileName(wstring filePath)
{
	wstring filePath_=standardizePathSeparator(filePath);
	wstring::size_type indexOfLastPathSeparator=filePath_.find_last_of(L'\\');
	wstring fileName=filePath;
	if(wstring::npos!=indexOfLastPathSeparator)
	{
		fileName=filePath.substr(indexOfLastPathSeparator+1);
	}
	return fileName;
}
bool isSameFileExtension(wstring ext1,wstring ext2)
{
	CString extension1=ext1.c_str();
	CString extension2=ext2.c_str();
	return extension1.CompareNoCase(extension2)==0;
}
wstring getGuidString()
{
	GUID guidStruct;
	HRESULT hResult= ::CoCreateGuid(&guidStruct);
	const UINT guidStringBufferLength=1024;
	OLECHAR guidStringBuffer[guidStringBufferLength];
	int guidStringLength= ::StringFromGUID2(guidStruct,guidStringBuffer,guidStringBufferLength);
	wstring guidString=guidStringBuffer;
	return guidString;
}
wstring getUniqueCompoundFileName()
{
	wstring guidString=getGuidString();
	guidString=guidString.substr(1,guidString.length()-2);
	while(guidString.find_first_of(L"-")!=wstring::npos)
	{
		guidString.erase(guidString.find_first_of(L"-"),1);
	}
	guidString.erase(31);
	return guidString;
}
#define ON_ERROR_RETURN(hResult) if (S_OK != hResult)\
{\
	return;\
}
void FileArchiver::importFile(wstring sourceFilePath,IStorage*pStg)
{
	IStream*pStmFrom=0;
	HRESULT hResult= ::SHCreateStreamOnFile(sourceFilePath.c_str(),READ_MODE,&pStmFrom);
	ON_ERROR_RETURN(hResult);
	IStream*pStmTo=0;
	wstring guid=getUniqueCompoundFileName();
	hResult=pStg->CreateStream(guid.c_str(),WRITE_MODE,0,0,&pStmTo);
	if(S_OK!=hResult)
	{
		pStmFrom->Release();
		return;
	}
	wstring fileName=getFileName(sourceFilePath);
	ULONG fileNameLength=(ULONG)fileName.size()*sizeof(fileName[0]),writeBytes=0;
	pStmTo->Write((const void*)&fileNameLength,sizeof(ULONG),&writeBytes);
	assert(sizeof(ULONG)==writeBytes);
	writeBytes=0;
	pStmTo->Write(fileName.c_str(),fileNameLength,&writeBytes);
	assert(fileNameLength==writeBytes);
	ULARGE_INTEGER ui1,ui2,ui3;
	hResult= ::IStream_Size(pStmFrom,&ui1);
	ON_ERROR_RETURN(hResult);
	hResult=pStmFrom->CopyTo(pStmTo,ui1,&ui2,&ui3);
	ON_ERROR_RETURN(hResult);
	hResult=pStmTo->Commit(STGC_CONSOLIDATE);
	ON_ERROR_RETURN(hResult);
	pStmFrom->Release();
	pStmTo->Release();
}
void FileArchiver::importDirectory(wstring sourceDirectoryPath,IStorage*pStg)
{
	sourceDirectoryPath=standardizeDirectoryPath(sourceDirectoryPath);
	WIN32_FIND_DATA findData;
	wstring findFolderPath=sourceDirectoryPath;
	findFolderPath.append(L"*");
	HANDLE findHandle= ::FindFirstFile(findFolderPath.c_str(),&findData);
	BOOL findResult=TRUE;
	while(INVALID_HANDLE_VALUE!=findHandle&&TRUE==findResult)
	{
		if(findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(findData.cFileName[0]!=L'.')
			{
				IStorage*pSubStg=0;
				wstring folderName=findData.cFileName;
				if(folderName.length()>31)
				{
					wstring guidName=getUniqueCompoundFileName();
					m_guidNameMap[guidName]=findData.cFileName;
					folderName=guidName;
				}
				HRESULT hResult=pStg->CreateStorage(folderName.c_str(),WRITE_MODE,0,0,&pSubStg);
				ON_ERROR_RETURN(hResult);
				wstring subSourceDirectoryPath=sourceDirectoryPath;
				subSourceDirectoryPath.append(findData.cFileName);
				subSourceDirectoryPath.append(L"\\");
				importDirectory(subSourceDirectoryPath,pSubStg);
				pSubStg->Release();
			}
		}
		else
		{
			wstring sourceFilePath=sourceDirectoryPath;
			sourceFilePath.append(findData.cFileName);
			importFile(sourceFilePath,pStg);
		}
		findResult= ::FindNextFile(findHandle,&findData);
	}
	::FindClose(findHandle);
}
void FileArchiver::exportStream(IStorage*pStg,wstring streamName,wstring destinationDirectoryPath)
{
	IStream*pStmFrom=0;
	HRESULT hResult=pStg->OpenStream(streamName.c_str(),0,READ_MODE,0,&pStmFrom);
	ON_ERROR_RETURN(hResult);
	ULONG fileNameLength=0,readBytes=0;
	pStmFrom->Read((void*)&fileNameLength,sizeof(ULONG),&readBytes);
	if(fileNameLength<=0)
	{
		pStmFrom->Release();
		return;
	}
	readBytes=0;
	wchar_t fileName[FILE_NAME_LENGTH]=
	{
		0
	};
	pStmFrom->Read((void*)fileName,fileNameLength,&readBytes);
	if(fileNameLength!=readBytes)
	{
		pStmFrom->Release();
		return;
	}
	IStream*pStmTo=0;
	destinationDirectoryPath=standardizeDirectoryPath(destinationDirectoryPath);
	hResult= ::SHCreateStreamOnFile(destinationDirectoryPath.append(fileName).c_str(),STGM_CREATE|WRITE_MODE,&pStmTo);
	ON_ERROR_RETURN(hResult);
	ULARGE_INTEGER ui1,ui2,ui3;
	hResult= ::IStream_Size(pStmFrom,&ui1);
	ON_ERROR_RETURN(hResult);
	hResult=pStmFrom->CopyTo(pStmTo,ui1,&ui2,&ui3);
	ON_ERROR_RETURN(hResult);
	hResult=pStmTo->Commit(STGC_CONSOLIDATE);
	ON_ERROR_RETURN(hResult);
	pStmFrom->Release();
	pStmTo->Release();
}
void FileArchiver::exportStorage(IStorage*pStg,wstring destinationDirectoryPath)
{
	if(!::PathIsDirectory(destinationDirectoryPath.c_str()))
	{
		::CreateDirectory(destinationDirectoryPath.c_str(),0);
	}
	IEnumSTATSTG*pEnum=0;
	HRESULT hResult=pStg->EnumElements(0,0,0,&pEnum);
	ON_ERROR_RETURN(hResult);
	STATSTG statStg;
	CString name;
	while(S_OK==pEnum->Next(1,&statStg,0))
	{
		name=statStg.pwcsName;
		::CoTaskMemFree(statStg.pwcsName);
		if(statStg.type==STGTY_STORAGE)
		{
			IStorage*pSubStg;
			hResult=pStg->OpenStorage(name,0,READ_MODE,0,0,&pSubStg);
			ON_ERROR_RETURN(hResult);
			wstring subFolderPath=standardizeDirectoryPath(destinationDirectoryPath);
			wstring guidName(name);
			if(m_guidNameMap.find(guidName)!=m_guidNameMap.end())
			{
				name=m_guidNameMap[guidName].c_str();
			}
			subFolderPath.append((LPCWSTR)name);
			subFolderPath.append(L"\\");
			exportStorage(pSubStg,subFolderPath);
			pSubStg->Release();
		}
		else if(statStg.type==STGTY_STREAM)
		{
			exportStream(pStg,(LPCWSTR)name,destinationDirectoryPath);
		}
	}
	pEnum->Release();
}
void FileArchiver::Unzip(wstring sourceFilePath,wstring destinationDirectoryPath)
{
	HRESULT hResult= ::StgIsStorageFile(sourceFilePath.c_str());
	ON_ERROR_RETURN(hResult);
	IStorage*pStg=0;
	hResult= ::StgOpenStorage(sourceFilePath.c_str(),0,READ_MODE,0,0,&pStg);
	ON_ERROR_RETURN(hResult);
	loadMap(pStg);
	exportStorage(pStg,destinationDirectoryPath);
	pStg->Release();
}
void FileArchiver::Zip(wstring sourceDirectoryPath,wstring destinationFilePath)
{
	IStorage*pStg;
	HRESULT hResult= ::StgCreateDocfile(destinationFilePath.c_str(),WRITE_MODE|STGM_CREATE,0,&pStg);
	ON_ERROR_RETURN(hResult);
	if(S_OK==hResult)
	{
		if(::PathIsDirectory(sourceDirectoryPath.c_str()))
		{
			importDirectory(sourceDirectoryPath,pStg);
			saveMap(pStg);
		}
	}
	pStg->Release();
}
void FileArchiver::loadMap(IStorage*pStg)
{
	m_guidNameMap.clear();
	IEnumSTATSTG*pEnum=0;
	HRESULT hResult=pStg->EnumElements(0,0,0,&pEnum);
	ON_ERROR_RETURN(hResult);
	STATSTG statStg;
	wstring name;
	while(S_OK==pEnum->Next(1,&statStg,0))
	{
		name=statStg.pwcsName;
		::CoTaskMemFree(statStg.pwcsName);
		if(statStg.type==STGTY_STREAM&&name==_T("GuidNameMap"))
		{
			IStream*pStmFrom=0;
			HRESULT hResult=pStg->OpenStream(name.c_str(),0,READ_MODE,0,&pStmFrom);
			ON_ERROR_RETURN(hResult);
			try
			{
				HRESULT hr=S_OK;
				while(hr==S_OK)
				{
					ULONG guidNameLength=0,readBytes=0;
					hr=pStmFrom->Read((void*)&guidNameLength,sizeof(ULONG),&readBytes);
					if(hr!=S_OK||readBytes==0)
					{
						break;
					}
					assert(sizeof(ULONG)==readBytes);
					assert(guidNameLength>0);
					readBytes=0;
					wchar_t guidName[FILE_NAME_LENGTH]=
					{
						0
					};
					hr=pStmFrom->Read((void*)guidName,guidNameLength,&readBytes);
					if(hr!=S_OK||readBytes==0)
					{
						break;
					}
					assert(guidNameLength==readBytes);
					ULONG fileNameLength=0;
					readBytes=0;
					hr=pStmFrom->Read((void*)&fileNameLength,sizeof(ULONG),&readBytes);
					if(hr!=S_OK||readBytes==0)
					{
						break;
					}
					assert(sizeof(ULONG)==readBytes);
					assert(fileNameLength>0);
					readBytes=0;
					wchar_t fileName[FILE_NAME_LENGTH]=
					{
						0
					};
					hr=pStmFrom->Read((void*)fileName,fileNameLength,&readBytes);
					if(hr!=S_OK||readBytes==0)
					{
						break;
					}
					assert(fileNameLength==readBytes);
					m_guidNameMap[guidName]=fileName;
				}
			}
			catch(const std::exception&)
			{
			}
			pStmFrom->Release();
		}
	}
	pEnum->Release();
}
void FileArchiver::saveMap(IStorage*pStg)
{
	IStream*pStmTo=0;
	HRESULT hResult=pStg->CreateStream(_T("GuidNameMap"),WRITE_MODE,0,0,&pStmTo);
	ON_ERROR_RETURN(hResult);
	for(map<wstring,wstring>::iterator it=m_guidNameMap.begin();it!=m_guidNameMap.end();it++)
	{
		wstring guidName=it->first;
		ULONG guidNameLength=(ULONG)guidName.size()*sizeof(guidName[0]),writeBytes=0;
		hResult=pStmTo->Write((const void*)&guidNameLength,sizeof(ULONG),&writeBytes);
		if(hResult!=S_OK)
		{
			break;
		}
		assert(sizeof(ULONG)==writeBytes);
		writeBytes=0;
		hResult=pStmTo->Write(guidName.c_str(),guidNameLength,&writeBytes);
		if(hResult!=S_OK)
		{
			break;
		}
		assert(guidNameLength==writeBytes);
		wstring fileName=it->second;
		ULONG fileNameLength=(ULONG)fileName.size()*sizeof(fileName[0]);
		writeBytes=0;
		hResult=pStmTo->Write((const void*)&fileNameLength,sizeof(ULONG),&writeBytes);
		if(hResult!=S_OK)
		{
			break;
		}
		assert(sizeof(ULONG)==writeBytes);
		writeBytes=0;
		hResult=pStmTo->Write(fileName.c_str(),fileNameLength,&writeBytes);
		if(hResult!=S_OK)
		{
			break;
		}
		assert(fileNameLength==writeBytes);
	}
	pStmTo->Release();
}
void FileArchiver_Zip(wchar_t*sourceDirectoryPath,wchar_t*destinationFilePath)
{
	FileArchiver::Zip(wstring(sourceDirectoryPath),wstring(destinationFilePath));
}
void FileArchiver_Unzip(wchar_t*sourceFilePath,wchar_t*destinationDirectoryPath)
{
	FileArchiver::Unzip(wstring(sourceFilePath),wstring(destinationDirectoryPath));
}
