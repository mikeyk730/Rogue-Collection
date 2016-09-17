//create_thing: Get a new item with a specified size
ITEM *create_item(int type, int which);
AGENT *create_agent();

//discard: Free up an item
void discard_item(ITEM *item);
void discard_agent(AGENT *item);
