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

int can_move_without_pickup()
{
    return version >= RV53A;
}

int version_has_invisible_stalker()
{
    return version < RV53A;
}

int has_double_haste_bug()
{
    return version < RV52A;
}

int version_has_wands()
{
    return version != RV36A;
}

int version_has_new_monsters()
{
    return version >= RV53A;
}

int get_zap_key()
{
    return (version < RV52A) ? 'p' : 'z';
}

int version_has_hallucination()
{
    return version >= RV53A;
}

int leather_is_rustproof()
{
    return version > RV36B; //todo:mdk: more places should check this
}

int get_repeat_message_key()
{
#ifndef ROGUE_COLLECTION
    if (version >= RV53A)
        return ctrl('p');
#endif
    return ctrl('r');
}

int get_redraw_key()
{
#ifndef ROGUE_COLLECTION
    if (version < RV53A)
        return ctrl('l');
else
    return ctrl('r');
#else
    return ctrl('e');
#endif
}

int new_weapon_protocol()
{
    return version >= RV54A;
}

int dynamic_inv_order()
{
    return version < RV53A;
}

int has_universal_identify_scroll()
{
    return version < RV53A;
}

int status_v1()
{
    return version < RV52A;
}

int status_v2()
{
    return version >= RV52A && version < RV53A;
}

int version_uses_new_strength()
{
    return version > RV36B;
}

int version_uses_old_weapon_stats()
{
    return version < RV52A;
}

int run_uses_f_prefix()
{
    return version < RV53A;
}

int new_ring_format()
{
    return version >= RV53A;
}

int read_in_corner()
{
    return version <= RV36B;
}

int rust_monster_level()
{
    return version < RV52A ? 8 : 7;
}

int has_hidden_passages()
{
    return version >= RV53A;
}

int eat_after_fainting()
{
    return version < RV36B;
}