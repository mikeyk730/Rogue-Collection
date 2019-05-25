//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include <cstring>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include "random.h"
#include "game_state.h"
#include "scrolls.h"
#include "monsters.h"
#include "pack.h"
#include "list.h"
#include "output_shim.h"
#include "io.h"
#include "main.h"
#include "misc.h"
#include "wizard.h"
#include "slime.h"
#include "level.h"
#include "weapons.h"
#include "things.h"
#include "hero.h"
#include "monster.h"
#include "ring.h"
#include "armor.h"

const char *c_set = "bcdfghjklmnpqrstvwxyz";
const char *v_set = "aeiou";

const char *laugh = "you hear maniacal laughter%s.";
const char *in_dist = " in the distance";

//random_char_in(): return random character in given string
char random_char_in(const char *string)
{
    return (string[rnd(strlen(string))]);
}

//getsyl(): generate a random syllable
char* getsyl()
{
    static char _tsyl[4];

    _tsyl[3] = 0;
    _tsyl[2] = random_char_in(c_set);
    _tsyl[1] = random_char_in(v_set);
    _tsyl[0] = random_char_in(c_set);
    return (_tsyl);
}

std::string GenerateScrollName()
{
    int nsyl;
    char *cp, *sp;
    int nwords;

    cp = prbuf;
    nwords = rnd(in_small_screen_mode() ? 3 : 4) + 2;
    while (nwords--)
    {
        nsyl = rnd(2) + 1;
        while (nsyl--)
        {
            sp = getsyl();
            if (&cp[strlen(sp)] > &prbuf[MAXNAME - 1]) { nwords = 0; break; }
            while (*sp) *cp++ = *sp++;
        }
        *cp++ = ' ';
    }
    *--cp = '\0';
    //I'm tired of thinking about this one so just in case .....
    prbuf[MAXNAME] = 0;
    std::string scroll_name(prbuf);
    if (scroll_name.back() == ' ')
        scroll_name.pop_back();

    return scroll_name;
}

//do_read_scroll: Read a scroll from the pack and do the appropriate thing
bool do_read_scroll()
{
    Item *item = get_item("read", SCROLL);
    if (!item)
        return false;

    Scroll* scroll = dynamic_cast<Scroll*>(item);
    if (!scroll) {
        //mdk: reading non-scroll counts as turn
        msg("there is nothing on it to read");
        return true;
    }

    ifterse("the scroll vanishes", "as you read the scroll, it vanishes");
    if (scroll == game->hero().get_current_weapon())
        game->hero().set_current_weapon(NULL);

    scroll->Read();

    look(true); //put the result of the scroll on the screen
    update_status_bar();
    scroll->call_it();

    //Get rid of the thing
    if (scroll->m_count > 1)
        scroll->m_count--;
    else {
        game->hero().m_pack.remove(scroll);
        delete(scroll);
    }
    return true;
}

int is_scare_monster_scroll(Item* item)
{
    return dynamic_cast<ScareMonster*>(item) != nullptr;
}


Scroll::Scroll() :
    Item(SCROLL, 0)
{
}

std::string Scroll::TypeName() const
{
    return "scroll";
}

std::string Scroll::InventoryName() const
{
    std::ostringstream ss;

    if (m_count == 1) {
        ss << "A scroll ";
    }
    else {
        ss << m_count << " scrolls ";
    }

    ItemCategory& scroll_info = *Category();

    if (scroll_info.is_discovered() || game->wizard().reveal_items())
        ss << "of " << scroll_info.name();
    else if (!scroll_info.guess().empty())
        ss << "called " << scroll_info.guess();
    else {
        std::string title = scroll_info.identifier();
        if (short_msgs() && title.length() > 17) {
            title = title.substr(0, 17);
        }
        ss << "titled '" << title << "'";
    }

    return ss.str();
}

bool Scroll::IsMagic() const
{
    return true;
}

bool Scroll::IsEvil() const
{
    return false;

}

int Scroll::Worth() const
{
    ItemCategory& scroll_info = *Category();

    int worth = scroll_info.worth();
    worth *= m_count;
    if (!scroll_info.is_discovered())
        worth /= 2;
    return worth;
}

void MonsterConfusion::Read()
{
    //Scroll of monster confusion.  Give him that power.
    game->hero().set_can_confuse(true);
    msg("your hands begin to glow red");
}

void MagicMapping::Read()
{
    //Scroll of magic mapping.
    discover();
    msg("oh, now this scroll has a map on it");
    game->level().show_map(false);

}

void HoldMonster::Read()
{
    //Hold monster scroll.  Stop all monsters within two spaces from chasing after the hero.
    int x, y;
    Monster* monster;

    //todo: move into level
    const int COLS = game->screen().columns();
    for (x = game->hero().position().x - 3; x <= game->hero().position().x + 3; x++) {
        if (x >= 0 && x < COLS) {
            for (y = game->hero().position().y - 3; y <= game->hero().position().y + 3; y++) {
                if ((y > 0 && y < maxrow()) && ((monster = game->level().monster_at({ x, y })) != NULL))
                {
                    monster->hold();
                }
            }
        }
    }
}

void Sleep::Read()
{
    //Scroll which makes you fall asleep
    discover();
    game->hero().increase_sleep_turns(rnd(SLEEP_TIME) + 4);
    msg("you fall asleep");
}

bool Sleep::IsEvil() const
{
    return true;
}

void EnchantArmor::Read()
{
    if (game->hero().get_current_armor() != NULL)
    {
        game->hero().get_current_armor()->enchant_armor();
        ifterse("your armor glows faintly", "your armor glows faintly for a moment");
    }
}

void Identify::Read()
{
    //Identify, let the rogue figure something out
    discover();
    msg("this scroll is an identify scroll");
    if (game->options.show_inventory_menu())
        more(" More ");
    whatis();
}

void ScareMonster::Read()
{
    //Reading it is a mistake and produces laughter at the poor rogue's boo boo.
    msg(laugh, short_msgs() ? "" : in_dist);
}

void FoodDetection::Read()
{
    //todo: move into level
    //Scroll of food detection
    byte discovered = false;

    for (auto it = game->level().items.begin(); it != game->level().items.end(); ++it)
    {
        Item* item = *it;
        if (item->m_type == FOOD)
        {
            discovered = true;
            game->screen().standout();
            game->screen().add_tile(item->position(), FOOD);
            game->screen().standend();
        }
        //as a bonus this will detect amulets as well
        else if (item->m_type == AMULET)
        {
            discovered = true;
            game->screen().standout();
            game->screen().add_tile(item->position(), AMULET);
            game->screen().standend();
        }
    }
    if (discovered) {
        discover();
        msg("your nose tingles as you sense food");
    }
    else
        ifterse("you hear a growling noise close by", "you hear a growling noise very close to you");

}

void TeleportationScroll::Read()
{
    //Scroll of teleportation: Make him disappear and reappear
    Room* original_room = game->hero().room();
    game->hero().teleport();
    if (original_room != game->hero().room())
        discover();
}

void EnchantWeapon::Read()
{
    Item* item = game->hero().get_current_weapon();
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (!weapon) {
        msg("you feel a strange sense of loss");
        return;
    }
    weapon->enchant_weapon();
}

void CreateMonster::Read()
{
    Agent* monster;
    Coord position;

    if (plop_monster(game->hero().position().y, game->hero().position().x, &position)) {
        monster = Monster::CreateMonster(randmonster(false, game->get_level()), &position, game->get_level());
    }
    else
        ifterse("you hear a faint cry of anguish", "you hear a faint cry of anguish in the distance");
}

bool CreateMonster::IsEvil() const
{
    return true;
}

void RemoveCurse::Read()
{
    if (game->hero().get_current_armor())
        game->hero().get_current_armor()->remove_curse();
    if (game->hero().get_current_weapon())
        game->hero().get_current_weapon()->remove_curse();
    if (game->hero().get_ring(LEFT))
        game->hero().get_ring(LEFT)->remove_curse();
    if (game->hero().get_ring(RIGHT))
        game->hero().get_ring(RIGHT)->remove_curse();

    ifterse("somebody is watching over you", "you feel as if somebody is watching over you");
}

void AggravateMonsters::Read()
{
    //This scroll aggravates all the monsters on the current level and sets them running towards the hero
    game->level().aggravate_monsters();
    ifterse("you hear a humming noise", "you hear a high pitched humming noise");
}

bool AggravateMonsters::IsEvil() const
{
    return true;
}

void BlankPaper::Read()
{
    msg("this scroll seems to be blank");
}

void VorpalizeWeapon::Read()
{
    //If he isn't wielding a weapon I get to chortle again!
    Item* item = game->hero().get_current_weapon();
    Weapon* weapon = dynamic_cast<Weapon*>(item);
    if (!weapon) {
        msg(laugh, short_msgs() ? "" : in_dist);
        return;
    }
    if (weapon->vorpalize())
        discover();
}

ItemCategory MonsterConfusion::info;
ItemCategory MagicMapping::info;
ItemCategory HoldMonster::info;
ItemCategory Sleep::info;
ItemCategory EnchantArmor::info;
ItemCategory Identify::info;
ItemCategory ScareMonster::info;
ItemCategory FoodDetection::info;
ItemCategory TeleportationScroll::info;
ItemCategory EnchantWeapon::info;
ItemCategory CreateMonster::info;
ItemCategory RemoveCurse::info;
ItemCategory AggravateMonsters::info;
ItemCategory BlankPaper::info;
ItemCategory VorpalizeWeapon::info;
