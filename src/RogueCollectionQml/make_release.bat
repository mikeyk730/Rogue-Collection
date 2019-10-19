xcopy ..\..\res\tilemap_v1.bmp ..\..\bin\staging\res\
xcopy ..\..\res\tilemap_v2.bmp ..\..\bin\staging\res\
xcopy ..\..\res\tilemap_v3.bmp ..\..\bin\staging\res\
xcopy ..\..\res\tilemap_v4.bmp ..\..\bin\staging\res\
xcopy ..\..\res\tilemap_text.bmp ..\..\bin\staging\res\
xcopy ..\..\res\boxy.bmp ..\..\bin\staging\res\
xcopy ..\..\res\fonts\Px437_IBM_VGA8.ttf ..\..\bin\staging\res\fonts\
xcopy ..\..\res\fonts\*.txt ..\..\bin\staging\res\fonts\
xcopy ..\..\res\sounds\*.wav ..\..\bin\staging\res\sounds\

xcopy ..\..\data\scrolls.dat ..\..\bin\staging\data\
xcopy ..\..\data\potions.dat ..\..\bin\staging\data\
xcopy ..\..\data\sticks.dat ..\..\bin\staging\data\
xcopy ..\..\data\rings.dat ..\..\bin\staging\data\

xcopy ..\..\src\RogueCollectionQml\gpl-3.0.txt ..\..\bin\staging\license\
copy ..\..\src\RogueVersions\Rogue_5_4_2\LICENSE.TXT  ..\..\bin\staging\license\unix-rogue.txt

xcopy ..\..\bin\build-RogueCollection-Static_32bit-Release\RogueCollection.exe ..\..\bin\staging\
xcopy ..\..\bin\build-RogueCollection-Static_32bit-Release\RoguePlugin\qmldir ..\..\bin\staging\RoguePlugin\
xcopy ..\..\bin\build-RetroRogueCollection-Static_32bit-Release\RetroRogueCollection.exe ..\..\bin\staging\
xcopy ..\..\bin\build-RetroRogueCollection-Static_32bit-Release\RoguePlugin\qmldir ..\..\bin\staging\RoguePlugin\ /F /R /Y /I
xcopy ..\..\bin\Win32\Release\Rogue_*.dll ..\..\bin\staging\
xcopy ..\..\bin\Win32\Release\Rogomatic*.dll ..\..\bin\staging\
xcopy ..\..\rogue.opt ..\..\bin\staging\
xcopy ..\..\docs\readme.md ..\..\bin\staging\
xcopy ..\..\readme.md ..\..\bin\staging\rlog\
