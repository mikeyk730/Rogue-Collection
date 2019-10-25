@echo off
echo Performing dry run:
echo -------------------
git clean -xdfn -e src/RogueCollectionQml/RetroRogueCollection.pro.user -e src/RogueCollectionQml/RogueCollection.pro.user -e src/RogueCollectionSdl/RogueCollection.vcxproj.user -e Rogue.VC.db -e Rogue.VC.VC.opendb
echo.
echo Command to remove files listed above:
echo -------------------------------------
echo git clean -xdf -e src/RogueCollectionQml/RetroRogueCollection.pro.user -e src/RogueCollectionQml/RogueCollection.pro.user -e src/RogueCollectionSdl/RogueCollection.vcxproj.user -e Rogue.VC.db -e Rogue.VC.VC.opendb