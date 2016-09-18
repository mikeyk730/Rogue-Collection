//_detach: Takes an item out of whatever linked list it might be in
void detach_item(std::list<Item*>& l, ITEM *item);

//_attach: add an item to the head of a list
void attach_item(std::list<Item*>& l, ITEM *item);

//_free_list: Throw the whole blamed thing away
void free_item_list(std::list<Item*>& l);

//_detach: Takes an item out of whatever linked list it might be in
void detach_agent(AGENT **list, AGENT *item);

//_attach: add an item to the head of a list
void attach_agent(AGENT **list, AGENT *item);

//_free_list: Throw the whole blamed thing away
void free_agent_list(AGENT **ptr);
