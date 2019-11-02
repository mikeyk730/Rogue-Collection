#include <map>
#include "text.h"

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
        { text_defeated,{ "you have defeated ", nullptr, nullptr, "defeated " } },
        { text_use, {
            "which object do you want to %s? (*for list) : ",
            "%s? (* for list): ",
            nullptr,
            "%s what? (* for list): "
        } }
    };
}

const char* get_text(GameState* game, text_id id)
{
    bool unix = game->options.unix_output();
    bool terse = game->options.terse();
    Text t = s_text_library[id];

    const char* m;
    if (unix) {
        m = terse ? t.unix_terse : t.unix_message;
    }
    else {
        m = terse ? t.terse : t.message;
    }

    return m ? m : t.message;
}
