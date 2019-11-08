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
        { text_more, { " More ", nullptr, "--More--", nullptr } },
        { text_defeated_monster, { "you have defeated ", nullptr, nullptr, "defeated " } },
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
        { text_sleep_trap, { "a strange white mist envelops you and you fall asleep", "a mist envelops you and you fall asleep", nullptr, nullptr } },
        { text_monster_detect_fail, {
            "you have a strange feeling for a moment.",
            "you have a strange feeling.",
            nullptr,
            nullptr
        } },
        { text_magic_detect_fail, {
            "you have a strange feeling for a moment, then it passes.", //no terse on unix
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
            "a dart whizzes by your ear and vanishes", nullptr, //no terse diff on unix
            "a small dart whizzes by your ear and vanishes", nullptr
        } },
        { text_remove_curse, { "you feel as if somebody is watching over you", "somebody is watching over you", nullptr, nullptr } },
        { text_use, {
            "which object do you want to %s? (*for list) : ",
            "%s? (* for list): ",
            nullptr,
            "%s what? (* for list): "
        } },
        { text_now_wielding, { "you are now wielding %s (%c)", "now wielding %s (%c)", nullptr, "wielding %s (%c)" } },
        { text_frozen, { "you are frozen by the %s", "you are frozen", nullptr, "you are frozen" } }, //Terse diff on unix
        { text_detect_food_fail, { "you hear a growling noise very close to you", "you hear a growling noise close by", nullptr, nullptr } }, //Terse, no msg on unix
        { text_create_monster_fail, { "you hear a faint cry of anguish in the distance", "you hear a faint cry of anguish", nullptr, nullptr } },
        { text_inventory_prompt, { "please specify a letter between 'a' and '%c'", "range is 'a' to '%c'", nullptr, "range is 'a' to '%c'" } },
        { text_faint, { "you feel very weak. You faint from lack of food", "you faint from lack of food", "you feel too weak from lack of food.  You faint", "You faint" } },
        { text_gain_item, { "you now have %s (%c)", "%s (%c)", nullptr, "%s (%c)" } },
        { text_scroll_dust, { "the scroll turns to dust as you pick it up.", "the scroll turns to dust.", nullptr, nullptr } },
        { text_wearing_ring, { "you are now wearing %s (%c)", "wearing %s (%c)", nullptr, "%s (%c)" } },
        { text_already_wearing_armor, { "you are already wearing some.  You'll have to take it off first.", "you are already wearing some.", nullptr, "you are already wearing some." } },
        { text_welcome, { "Hello %s.  Welcome to the Dungeons of Doom.", "Hello %s.", nullptr, nullptr } },
        { text_name_used_item_prompt, { "what do you want to call it? ", "call it? ", nullptr, "Call it: " } },
        { text_go_up_stairs, { "you feel a wrenching sensation in your gut", "you feel a wrenching sensation", nullptr, nullptr } },
        { text_identify_item, { "Which item do you wish to inventory: ", "Item: ", nullptr, "Item: " } },
        { text_maniacal_laughter, { "you hear maniacal laughter in the distance.", "you hear maniacal laughter.", nullptr, nullptr } },
        { text_vorpalize_weapon, { "your %s gives off a flash of intense white light", "your %s gives off a flash", nullptr, nullptr } },
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
