//save_game: Implement the "save game" command
save_game();

//Save: Determine the entire data area that needs to be saved, Open save file, first write in to save file a header that dimensions the data area that will be saved, and then dump data area determined previous to opening file.
save_ds(char *savename);

//Restore: Open saved data file, read in header, and determine how much data area is going to be restored.
//Close save data file, Allocate enough data space so that open data file information will be stored outside the data area that will be restored.
//Now reopen data save file, skip header, dump into memory all saved data.

restore(char *savefile);