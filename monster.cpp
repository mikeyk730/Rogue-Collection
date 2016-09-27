#include "monster.h"
#include "monster.h"
#include "monsters.h"

std::string Monster::get_name()
{
    return get_monster_name(type);
}
