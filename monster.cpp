#include "monster.h"
#include "monster.h"
#include "monsters.h"

std::string Monster::get_name()
{
    return get_monster_name(type);
}

void Monster::reveal_disguise() {
    disguise = type;
}

void Monster::set_dirty(bool enable) {
    value = enable ? 1 : 0;
}

bool Monster::is_dirty() {
    return value == 1;
}

//start_run: Set a monster running after something or stop it from running (for when it dies)
void Monster::start_run()
{
    //Start the beastie running
    set_running(true);
    set_is_held(false);
    set_destination();
}
