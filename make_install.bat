@echo off
echo This file assumes you've built the following:
echo  - Rogue.sln in Release
echo  - RogueCollection.pro to bin\build-RogueCollection-Static_32bit-Release
echo  - RetroRogueCollection.pro to bin\build-RetroRogueCollection-Static_32bit-Release

xcopy res\tilemap_v1.bmp bin\RogueCollection\res\ /F /D
xcopy res\tilemap_v2.bmp bin\RogueCollection\res\ /F /D
xcopy res\tilemap_v3.bmp bin\RogueCollection\res\ /F /D
xcopy res\tilemap_v4.bmp bin\RogueCollection\res\ /F /D
xcopy res\tilemap_text.bmp bin\RogueCollection\res\ /F /D
xcopy res\boxy.bmp bin\RogueCollection\res\ /F /D
xcopy res\fonts\Px437_IBM_VGA8.ttf bin\RogueCollection\res\fonts\ /F /D
xcopy res\fonts\*.txt bin\RogueCollection\res\fonts\ /F /D
xcopy res\sounds\*.wav bin\RogueCollection\res\sounds\ /F /D

xcopy data\scrolls.dat bin\RogueCollection\data\ /F /D
xcopy data\potions.dat bin\RogueCollection\data\ /F /D
xcopy data\sticks.dat bin\RogueCollection\data\ /F /D
xcopy data\rings.dat bin\RogueCollection\data\ /F /D

xcopy src\RogueCollectionQml\gpl-3.0.txt bin\RogueCollection\license\ /F /D
xcopy src\RogueVersions\Rogue_5_4_2\LICENSE.TXT  bin\RogueCollection\license\unix-rogue.txt /F /D

xcopy bin\build-RogueCollection-Static_32bit-Release\RogueCollection.exe bin\RogueCollection\ /F /D
xcopy src\RogueCollectionQml\Rogomatic.bat bin\RogueCollection\ /F /D
xcopy bin\build-RogueCollection-Static_32bit-Release\RoguePlugin\qmldir bin\RogueCollection\RoguePlugin\ /F /D
xcopy bin\build-RetroRogueCollection-Static_32bit-Release\RetroRogueCollection.exe bin\RogueCollection\ /F /D
xcopy src\RogueCollectionQml\RetroRogomatic.bat bin\RogueCollection\ /F /D
xcopy bin\build-RetroRogueCollection-Static_32bit-Release\RoguePlugin\qmldir bin\RogueCollection\RoguePlugin\ /F /D /R /Y /I
xcopy bin\Win32\Release\Rogue_*.dll bin\RogueCollection\ /F /D
xcopy bin\Win32\Release\Rogomatic*.dll bin\RogueCollection\ /F /D
xcopy rogue.opt bin\RogueCollection\ /F /D
xcopy docs\readme.md bin\RogueCollection\ /F /D
mkdir bin\RogueCollection\rlog\
