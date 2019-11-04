all:
	mkdir -p ../build/release/
	mkdir -p ../build/release/res/
	cp -R ../res/* ../build/release/res/
	mkdir -p ../build/release/data/
	cp -R ../data/* ../build/release/data/
	mkdir -p ../build/release/rlog/
	cp -R ../rogue.opt ../build/release/
	cp -R ../docs/readme.md ../build/release/
	$(MAKE) -C MyCurses
	$(MAKE) -C RogueVersions/Rogue_PC_Core
	$(MAKE) -C RogueVersions/Rogue_PC_1_48
	$(MAKE) -C RogueVersions/Rogue_3_6_3
	$(MAKE) -C RogueVersions/Rogue_5_2_1
	$(MAKE) -C RogueVersions/Rogue_5_3
	$(MAKE) -C RogueVersions/Rogue_5_4_2
	$(MAKE) -C Rogomatic
	$(MAKE) -C Shared/Frontend
	$(MAKE) -C RogueCollectionSdl

clean:
	$(MAKE) -C MyCurses clean
	$(MAKE) -C RogueVersions/Rogue_PC_Core clean
	$(MAKE) -C RogueVersions/Rogue_PC_1_48 clean
	$(MAKE) -C RogueVersions/Rogue_3_6_3 clean
	$(MAKE) -C RogueVersions/Rogue_5_2_1 clean
	$(MAKE) -C RogueVersions/Rogue_5_3 clean
	$(MAKE) -C RogueVersions/Rogue_5_4_2 clean
	$(MAKE) -C Rogomatic clean
	$(MAKE) -C Shared/Frontend clean
	$(MAKE) -C RogueCollectionSdl clean
