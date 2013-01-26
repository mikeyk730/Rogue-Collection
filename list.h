//_detach: Takes an item out of whatever linked list it might be in
void _detach(THING **list, THING *item);

//_attach: add an item to the head of a list
void _attach(THING **list, THING *item);

//_free_list: Throw the whole blamed thing away
void _free_list(THING **ptr);

#define attach(a,b)     _attach(&a,b)
#define detach(a,b)     _detach(&a,b)
#define free_list(a)    _free_list(&a)
