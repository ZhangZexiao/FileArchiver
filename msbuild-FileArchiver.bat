rem call "%vs140comntools%vsvars32.bat"
set path=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin;%path%
msbuild /p:Configuration=Debug /t:rebuild FileArchiver.sln
msbuild /p:Configuration=Release /t:rebuild FileArchiver.sln
pause
