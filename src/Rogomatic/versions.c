#include "globals.h"

int version_has_arrow_bug()
{
    return version <= RV36B;
}

int striking_takes_2_charges()
{
    return version >= RV52A;
}

int can_step_on_scare_monster_if_inv_full()
{
    return version < RV53A;
}

int version_supports_move_without_pickup()
{
    return version >= RV53A;
}

int version_has_invisible_stalker()
{
    return version < RV53A;
}

int version_has_double_haste_bug()
{
    return version < RV52A;
}

int version_has_wands()
{
    return version != RV36A;
}