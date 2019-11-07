#include <map>
#include "text.h"
#include "rogue.h"

namespace
{
    struct Text
    {
        const char* message;
        const char* terse;
        const char* unix_message;
        const char* unix_terse;
    };

    std::map<text_id, Text> s_text_library =
    {
        { text_defeated_monster,{ "you have defeated ", nullptr, nullptr, "defeated " } },
        { text_found_gold, { "you found %d gold pieces", nullptr, nullptr, "%d gold pieces" } },
        { text_scroll_vanishes, { "as you read the scroll, it vanishes", "the scroll vanishes", nullptr, nullptr } },
        { text_hungry, { "you are starting to get hungry", nullptr, nullptr, "getting hungry" } },
        { text_aggravate_monsters, { "you hear a high pitched humming noise", "you hear a humming noise", nullptr, nullptr } },
        { text_item_vanishes, { "the %s vanishes as it hits the ground.", "the %s vanishes.", nullptr, nullptr } },
        { text_restore_strength, { "hey, this tastes great.  It makes you feel warm all over", "you feel warm all over", nullptr, nullptr } },
        { text_remove_armor, { "you used to be wearing %c) %s", nullptr, nullptr, "was wearing %c) %s" } },
        { text_found_trap, { "you found %s", nullptr, nullptr, "%s" } },
        { text_wear_armor, { "you are now wearing %s", nullptr, nullptr, "wearing %s" } },
        { text_enchant_armor, { "your armor glows faintly for a moment", "your armor glows faintly", nullptr, nullptr } },
        { text_enchant_weapon, { "your %s glows blue for a moment", "your %s glows blue", nullptr, nullptr } },
        { text_missile_vanishes, { "the missile vanishes with a puff of smoke", nullptr, nullptr, "missile vanishes" } },
        { text_monster_detect_fail, {
            "you have a strange feeling for a moment.",
            "you have a strange feeling.",
            nullptr,
            nullptr
        } },
        { text_magic_detect_fail, {
            "you have a strange feeling for a moment, then it passes.",
            "you have a strange feeling for a moment.",
            nullptr,
            nullptr
        } },
        { text_sick, {
            "you feel very sick.", nullptr,
            "you feel very sick now", nullptr
        } },
        { text_gain_strength, {
            "you feel stronger. What bulging muscles!", nullptr,
            "You feel stronger, now.  What bulging muscles!", nullptr
        } },
        { text_was_called, { "Was called \"%s\"", nullptr, nullptr, "Called \"%s\"" } },
        { text_call_it, { "what do you want to call it? ", nullptr, nullptr, "Call it: " } },
        { text_bite, {
            "you feel a bite in your leg and now feel weaker",
            "you feel a bite in your leg",
            nullptr,
            "a bite has weakened you"
        } },
        { text_dart_hit, {
            "a dart just hit you in the shoulder", nullptr,
            "A small dart just hit you in the shoulder", nullptr
        } },
        { text_dart_miss, {
            "a dart whizzes by your ear and vanishes", nullptr,
            "a small dart whizzes by your ear and vanishes", nullptr
        } },
        { text_remove_curse, { "you feel as if somebody is watching over you", "somebody is watching over you", nullptr, nullptr } },
        { text_use, {
            "which object do you want to %s? (*for list) : ",
            "%s? (* for list): ",
            nullptr,
            "%s what? (* for list): "
        } }
    };
}

const char* get_text(text_id id)
{
    bool unix = game->options.unix_output();
    bool terse = game->options.terse();
    Text t = s_text_library[id];

    const char* m;
    if (unix) {
        m = terse && t.unix_terse ? t.unix_terse : t.unix_message;
    }
    else {
        m = terse ? t.terse : t.message;
    }

    return m ? m : t.message;
}
