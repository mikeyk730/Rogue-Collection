@if [%1] == [] goto getgame
@set id=%1
@goto play

:getgame
@echo Run Rogomatic for which game?
@echo.
@echo a) PC Rogue 1.48
@echo b) PC Rogue 1.1
@echo c) Unix Rogue 5.4.2
@echo d) Unix Rogue 5.3
@echo e) Unix Rogue 5.2.1
@echo f) Unix Rogue 3.6.3
@echo.
@set /p "id=Enter letter: "
@goto play

:play
RetroRogueCollection.exe --rogomatic %id%
