#include "../FileArchiver/FileArchiver.h"
int main(int argc,char**argv)
{
	enum
	{
		GOOD=0,BAD_ARGUMENT,FILE_NOT_EXISTS
	};
	if(argc!=3)
	{
		printf("bad argument!\n");
		return BAD_ARGUMENT;
	}
	printf("%s, %s, %s\n",argv[0],argv[1],argv[2]);
	string source=argv[1];
	string destination=argv[2];
	wstring wsource,wdestination;
	wsource.assign(source.begin(),source.end());
	wdestination.assign(destination.begin(),destination.end());
	DWORD sourceAttributes=GetFileAttributesW(wsource.c_str());
	DWORD destinationAttributes=GetFileAttributesW(wdestination.c_str());
	if(sourceAttributes==INVALID_FILE_ATTRIBUTES&&destinationAttributes==INVALID_FILE_ATTRIBUTES)
	{
		printf("file not exists!\n");
		return FILE_NOT_EXISTS;
	}
	bool isZip=sourceAttributes==FILE_ATTRIBUTE_DIRECTORY&&destinationAttributes==INVALID_FILE_ATTRIBUTES;
	bool isUnzip=sourceAttributes==FILE_ATTRIBUTE_ARCHIVE&&(destinationAttributes==FILE_ATTRIBUTE_DIRECTORY||destinationAttributes==INVALID_FILE_ATTRIBUTES);
	system("pause");
	if(!isZip&& !isUnzip)
	{
		printf("bad argument!\n");
		return BAD_ARGUMENT;
	}
	if(isZip)
	{
		printf("zip!\n");
		FileArchiver::Zip(wsource,wdestination);
	}
	if(isUnzip)
	{
		printf("unzip!\n");
		FileArchiver::Unzip(wsource,wdestination);
	}
	printf("done!\n");
	return GOOD;
}
