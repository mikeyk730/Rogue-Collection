set QtStaticRoot="C:\dev\Qt\Qtv5.7.0_X86_MSVC2015_MD\qtbase"
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

msbuild Rogue.sln /t:Build /p:Configuration=Release /m

mkdir bin\build-RogueCollection-Static_32bit-Release
cd bin\build-RogueCollection-Static_32bit-Release
%QtStaticRoot%\bin\qmake.exe ..\..\src\RogueCollectionQml\RogueCollection.pro -spec win32-msvc2015 "CONFIG+=release" "CONFIG+=static" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all
nmake
cd ..\..

mkdir bin\build-RetroRogueCollection-Static_32bit-Release
cd bin\build-RetroRogueCollection-Static_32bit-Release
%QtStaticRoot%\bin\qmake.exe ..\..\src\RogueCollectionQml\RetroRogueCollection.pro -spec win32-msvc2015 "CONFIG+=release" "CONFIG+=static" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all
nmake
cd ..\..

call make_install.bat
