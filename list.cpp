//Functions for dealing with linked lists of goodies
//Functions with names starting with an "_" have companion #defines in rogue.h which take the address of the first argument and pass it on.
//list.c      1.4 (A.I. Design) 12/5/85

#include "rogue.h"
#include "list.h"
#include "io.h"
#include "misc.h"
#include "thing.h"

void detach_item(std::list<Item*>& l, ITEM *item)
{
    l.remove(item);
}

void attach_item(std::list<Item*>& l, ITEM *item)
{
    l.push_front(item);
}

void free_item_list(std::list<Item*>& l)
{
    for (auto it = level_items.begin(); it != level_items.end(); ++it){
        discard_item(*it);
    }
    l.clear();
}

//_detach: Takes an agent out of whatever linked list it might be in
void detach_agent(AGENT **list, AGENT *agent)
{
  if (*list==agent) 
    *list = next(agent);
  if (prev(agent)!=NULL) 
    agent->l_prev->l_next = next(agent);
  if (next(agent)!=NULL) 
    agent->l_next->l_prev = prev(agent);
  agent->l_next = NULL;
  agent->l_prev = NULL;
}

//_attach: add an agent to the head of a list
void attach_agent(AGENT **list, AGENT *agent)
{
  if (*list!=NULL) {
    agent->l_next = *list; 
    (*list)->l_prev = agent;
    agent->l_prev = NULL;
  }
  else {
    agent->l_next = NULL; 
    agent->l_prev = NULL;
  }
  *list = agent;
}

//_free_list: Throw the whole blamed thing away
void free_agent_list(AGENT **ptr)
{
  AGENT *agent;
  while (*ptr!=NULL) {
    agent = *ptr;
    *ptr = next(agent); 
    discard_agent(agent);
  }
}
