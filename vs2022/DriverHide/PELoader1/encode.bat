set "projectpath=%cd%"
cd ../
set "preProjectpath=%cd%"
cd %projectpath%
set "SignFullPath=%preProjectpath%/x64/Debug/PELoaderTest.sys"
Build.exe %SignFullPath%

