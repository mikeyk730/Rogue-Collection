@echo off
echo This file assumes you've built the following:
echo  - Rogue.sln in Release
echo  - RogueCollection.pro to bin\build-RogueCollection-Static_32bit-Release
echo  - RetroRogueCollection.pro to bin\build-RetroRogueCollection-Static_32bit-Release

xcopy res\tilemap_v1.bmp bin\RogueCollection\res\
xcopy res\tilemap_v2.bmp bin\RogueCollection\res\
xcopy res\tilemap_v3.bmp bin\RogueCollection\res\
xcopy res\tilemap_v4.bmp bin\RogueCollection\res\
xcopy res\tilemap_text.bmp bin\RogueCollection\res\
xcopy res\boxy.bmp bin\RogueCollection\res\
xcopy res\fonts\Px437_IBM_VGA8.ttf bin\RogueCollection\res\fonts\
xcopy res\fonts\*.txt bin\RogueCollection\res\fonts\
xcopy res\sounds\*.wav bin\RogueCollection\res\sounds\

xcopy data\scrolls.dat bin\RogueCollection\data\
xcopy data\potions.dat bin\RogueCollection\data\
xcopy data\sticks.dat bin\RogueCollection\data\
xcopy data\rings.dat bin\RogueCollection\data\

xcopy src\RogueCollectionQml\gpl-3.0.txt bin\RogueCollection\license\
copy src\RogueVersions\Rogue_5_4_2\LICENSE.TXT  bin\RogueCollection\license\unix-rogue.txt

xcopy bin\build-RogueCollection-Static_32bit-Release\RogueCollection.exe bin\RogueCollection\
xcopy bin\build-RogueCollection-Static_32bit-Release\RoguePlugin\qmldir bin\RogueCollection\RoguePlugin\
xcopy bin\build-RetroRogueCollection-Static_32bit-Release\RetroRogueCollection.exe bin\RogueCollection\
xcopy bin\build-RetroRogueCollection-Static_32bit-Release\RoguePlugin\qmldir bin\RogueCollection\RoguePlugin\ /F /R /Y /I
xcopy bin\Win32\Release\Rogue_*.dll bin\RogueCollection\
xcopy bin\Win32\Release\Rogomatic*.dll bin\RogueCollection\
xcopy rogue.opt bin\RogueCollection\
xcopy docs\readme.md bin\RogueCollection\
xcopy readme.md bin\RogueCollection\rlog\
