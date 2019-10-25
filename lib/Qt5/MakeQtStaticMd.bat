CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86

set "qtver=5.7.0"
set "qttag=v%qtver%"
set "prefix=Qt"
set "suffix=_src"
set "buildSuffix=X86_MSVC2015_MD"
set "folder=%prefix%%suffix%"
if "%buildSuffix%" == "" set compilefolder=%prefix%%qtver%
if NOT "%buildSuffix%" == "" set compilefolder=%prefix%%qtver%_%buildSuffix%

if "%qtver%" == "" goto :leave

echo Press enter to checkout code (~4Gb)
pause
CALL git clone https://code.qt.io/qt/qt5.git %folder%
CALL cd %folder%
CALL git checkout .
if NOT %ERRORLEVEL% == 0 goto :errorcheckout
CALL perl init-repository --module-subset=default,-qtwebengine
CALL git checkout %qttag%
call git submodule update --checkout --recursive
CALL cd ..\

echo Press enter to configure build. Manually apply patches to source before continuing
pause
set PATH=%cd%\%folder%\bin;%PATH%
set QTDIR=%cd%\%folder%\qtbase
mkdir %compilefolder%
call cd %compilefolder%
set CL=/MP
CALL ..\%folder%\configure -confirm-license -release -opensource -platform win32-msvc2015 -opengl desktop -static -nomake examples -nomake tests -mp -qt-zlib -qt-pcre -qt-libpng -qt-libjpeg

REM Configure will tell you if you need to run "nmake" or both "nmake" and "nmake install"
echo Press enter to compile build (~10Gb)
pause
CALL nmake

REM echo Press enter to install build
REM pause
REM CALL nmake install

REM echo Press enter to copy configuration to build output
REM pause
REM copy qt.conf C:\Qt\Qt-%qver%\bin\
REM echo Check build output in C:\Qt\Qt-%qver%

echo Setup kit and pass "CONFIG+=static" into qmake in your project
pause

:leave
if "%qtver%" == "" echo Please enter a version as the first parameter, and the build directory suffix as the second parameter
exit /b 1

:errorcheckout
echo Checkout error... The version you chose does not exist exiting
exit /b 1