xcopy ..\..\res\tilemap_v1.bmp .\staging\res\
xcopy ..\..\res\tilemap_v2.bmp .\staging\res\
xcopy ..\..\res\tilemap_v3.bmp .\staging\res\
xcopy ..\..\res\tilemap_v4.bmp .\staging\res\
xcopy ..\..\res\tilemap_text.bmp .\staging\res\
xcopy ..\..\res\boxy.bmp .\staging\res\
xcopy ..\..\res\fonts\Px437_IBM_VGA8.ttf .\staging\res\fonts\
xcopy ..\..\res\fonts\*.txt .\staging\res\fonts\
xcopy ..\..\res\sounds\*.wav .\staging\res\sounds\

xcopy ..\..\data\scrolls.dat .\staging\data\
xcopy ..\..\data\potions.dat .\staging\data\
xcopy ..\..\data\sticks.dat .\staging\data\
xcopy ..\..\data\rings.dat .\staging\data\

xcopy ..\..\src\RogueCollectionQml\gpl-3.0.txt .\staging\license\
copy ..\..\src\RogueVersions\Rogue_5_4_2\LICENSE.TXT  .\staging\license\unix-rogue.txt

xcopy ..\..\src\build-RogueCollection-Static_32bit-Release\RogueCollection.exe .\staging\
xcopy ..\..\src\build-RogueCollection-Static_32bit-Release\RoguePlugin\qmldir .\staging\RoguePlugin\
xcopy ..\..\src\build-RetroRogueCollection-Static_32bit-Release\RetroRogueCollection.exe .\staging\
xcopy ..\..\src\build-RetroRogueCollection-Static_32bit-Release\RoguePlugin\qmldir .\staging\RoguePlugin\
xcopy ..\..\bin\Win32\Release\Rogue_*.dll .\staging\
xcopy ..\..\bin\Win32\Release\Rogomatic*.dll .\staging\
xcopy ..\..\rogue.opt .\staging\
xcopy ..\..\docs\readme.md .\staging\

cd staging
mkdir rlog
cd ..
