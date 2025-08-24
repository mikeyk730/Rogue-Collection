@rem This script builds the repo into 'bin\RogueCollection'. It assumes the following is installed:
@rem
@rem   Visual Studio 2022 Community     C:\Program Files\Microsoft Visual Studio\2022\Community
@rem   Windows 10 SDK                   C:\Program Files (x86)\Windows Kits\10\Lib\10.0.22621.0\um
@rem   Qt 5.7.0                         C:\Qt\Qt5.7.0
@rem
@rem If Qt is installed with qt-opensource-windows-x86-msvc2015-5.7.0.exe from qt.io, the Qt dlls
@rem from 'C:\Qt\Qt5.7.0\5.7\msvc2015\bin' need to be distributed alongside RogueCollection.exe and
@rem RetroRogueCollection.exe
@rem
@rem To statically link Qt into the executables, you must build Qt from source, according to
@rem 'lib\Qt5\MakeQtStaticMd.bat'

@rem This variable controls whether the script links against static or dynamic Qt libs
set MdkQtStatic=1

@rem Static Qt libs are built from source
if defined MdkQtStatic set QtMsvcRoot="C:\dev\Qt\Qtv5.7.0_X86_MSVC2015_MD\qtbase"
@rem Dynamic Qt libs are from installer
if not defined MdkQtStatic rem set QtMsvcRoot=C:\Qt\Qt5.7.0\5.7\msvc2015
@rem Qt Creator is from installer
set QtCreatorRoot=C:\Qt\Qt5.7.0\Tools\QtCreator

if not defined VSINSTALLDIR (
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
    if %ERRORLEVEL% NEQ 0 (
        echo Visual Studio not found
        exit /B %ERRORLEVEL%
    )
)

msbuild Rogue.sln /t:Build /p:Configuration=Release /m
if %ERRORLEVEL% NEQ 0 (
    echo Failed to build solution
    exit /B %ERRORLEVEL%
)

mkdir bin\build-RogueCollection-Static_32bit-Release
pushd bin\build-RogueCollection-Static_32bit-Release
%QtMsvcRoot%\bin\qmake.exe ..\..\src\RogueCollectionQml\RogueCollection.pro -spec win32-msvc2015 "CONFIG+=release" "CONFIG+=static"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run qmake, Rogue Collection
    exit /B %ERRORLEVEL%
)
%QtCreatorRoot%\bin\jom.exe qmake_all
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run jom, Rogue Collection
    exit /B %ERRORLEVEL%
)
nmake
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run nmake, Rogue Collection
    exit /B %ERRORLEVEL%
)
popd

mkdir bin\build-RetroRogueCollection-Static_32bit-Release
pushd bin\build-RetroRogueCollection-Static_32bit-Release
%QtMsvcRoot%\bin\qmake.exe ..\..\src\RogueCollectionQml\RetroRogueCollection.pro -spec win32-msvc2015 "CONFIG+=release" "CONFIG+=static"
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run qmake, Retro Rogue Collection
    exit /B %ERRORLEVEL%
)
%QtCreatorRoot%/bin/jom.exe qmake_all
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run jom, Retro Rogue Collection
    exit /B %ERRORLEVEL%
)
nmake
if %ERRORLEVEL% NEQ 0 (
    echo Failed to run nmake, Retro Rogue Collection
    exit /B %ERRORLEVEL%
)
popd

call make_install.bat

@rem If building with Qt dynamic libs, we need the .dlls
if not defined MdkQtStatic xcopy %QtMsvcRoot%\bin\*.dll bin\RogueCollection\ /F /D
