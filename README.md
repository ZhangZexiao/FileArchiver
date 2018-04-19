# FileArchiver
Zip a directory into an archive file OR Unzip an archive file to a directory.

## How to use "FileArchiver.dll" to zip and unzip:
1. Add NuGet package to project.

2. Add directive: #include "FileArchiver.h" to source file.

3. Call FileArchiver::Zip(source, destination); OR FileArchiver::Unzip(source, destination);

## How to use FileArchiver static library:
1. Add NuGet package to project.

2. Define preprocessor macro FILE_ARCHIVER_STATIC before including "FileArchiver.h" in your source file.

3. Link with static library: FileArchiverStatic.lib

## NuGet Repository

https://api.nuget.org/v3/index.json

## Package ID for DLL

FileArchiver_win_intel64_v140

## Package ID for static LIB

FileArchiver_win_intel64_v140_static
