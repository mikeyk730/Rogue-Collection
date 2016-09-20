//Functions for dealing with linked lists of goodies
//Functions with names starting with an "_" have companion #defines in rogue.h which take the address of the first argument and pass it on.
//list.c      1.4 (A.I. Design) 12/5/85

#include "rogue.h"
#include "list.h"
#include "io.h"
#include "misc.h"
#include "thing.h"

void detach_item(std::list<Item*>& l, Item *item)
{
    l.remove(item);
}

void attach_item(std::list<Item*>& l, Item *item)
{
    l.push_front(item);
}

void free_item_list(std::list<Item*>& l)
{
    for (auto it = l.begin(); it != l.end(); ++it){
        discard_item(*it);
    }
    l.clear();
}

//_detach: Takes an agent out of whatever linked list it might be in
void detach_agent(std::list<Agent*>& l, Agent *agent)
{
    l.remove(agent);
}

//_free_list: Throw the whole blamed thing away
void free_agent_list(std::list<Agent*>& l)
{
    for (auto it = l.begin(); it != l.end(); ++it){
        discard_agent(*it);
    }
    l.clear();
}
