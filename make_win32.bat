CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

msbuild Rogue.sln /t:Build /p:Configuration=Release /m

mkdir bin\build-RogueCollection-Static_32bit-Release
cd bin\build-RogueCollection-Static_32bit-Release
C:\Qt\Qt-5.9.8-static-x86-md\bin\qmake.exe ..\..\src\RogueCollectionQml\RogueCollection.pro -spec win32-msvc "CONFIG+=release" "CONFIG+=static" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all
nmake
cd ..\..

mkdir bin\build-RetroRogueCollection-Static_32bit-Release
cd bin\build-RetroRogueCollection-Static_32bit-Release
C:\Qt\Qt-5.9.8-static-x86-md\bin\qmake.exe ..\..\src\RogueCollectionQml\RetroRogueCollection.pro -spec win32-msvc "CONFIG+=release" "CONFIG+=static" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all
nmake
cd ..\..

call make_install.bat
