//Read a scroll and let it happen
//scrolls.c   1.4 (AI Design) 12/14/84

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include "random.h"
#include "item_class.h"
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
#include "rings.h"
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

template<typename T>
Item* createInstance() { return new T; }

template<typename T>
void setInfo(const std::string& name, const std::string& identifier, int worth) {
    T::info.name(name);
    T::info.identifier(identifier);
    T::info.worth(worth);
}

template<typename T>
ItemCategory& getName()
{
    return T::info;
}

struct map_entry
{
    Item*(*creator)();
    void(*setter)(const std::string&, const std::string&, int);
    ItemCategory&(*getter)();
};

struct vector_entry
{
    int prob;
    Item*(*create)();
    ItemCategory&(*info)();
};

#define ITEM_MAP_ENTRY(Type) {&createInstance<Type>, &setInfo<Type>, &getName<Type>}

std::map<std::string, map_entry> s_scrolls_type = {
    { "S_CONFUSE", ITEM_MAP_ENTRY(MonsterConfusion) },
    { "S_MAP", ITEM_MAP_ENTRY(MagicMapping) },
    { "S_HOLD", ITEM_MAP_ENTRY(HoldMonster) },
    { "S_SLEEP", ITEM_MAP_ENTRY(Sleep) },
    { "S_ARMOR", ITEM_MAP_ENTRY(EnchantArmor) },
    { "S_IDENT", ITEM_MAP_ENTRY(Identify) },
    { "S_SCARE", ITEM_MAP_ENTRY(ScareMonster) },
    { "S_GFIND", ITEM_MAP_ENTRY(FoodDetection) },
    { "S_TELEP", ITEM_MAP_ENTRY(Teleportation) },
    { "S_ENCH", ITEM_MAP_ENTRY(EnchantWeapon) },
    { "S_CREATE", ITEM_MAP_ENTRY(CreateMonster) },
    { "S_REMOVE", ITEM_MAP_ENTRY(RemoveCurse) },
    { "S_AGGR", ITEM_MAP_ENTRY(AggravateMonsters) },
    { "S_NOP", ITEM_MAP_ENTRY(BlankPaper) },
    { "S_VORPAL", ITEM_MAP_ENTRY(VorpalizeWeapon) },
};

std::vector<vector_entry> s_scrolls;

int NumScrollTypes()
{
    return s_scrolls.size();
}

Item* CreateScroll()
{
    int r = rnd(100);
    for (size_t i = 0; i < s_scrolls.size(); ++i)
    {
        if (r < s_scrolls[i].prob) {
            return s_scrolls[i].create();
        }
    }
    throw std::runtime_error("Failed to create scroll");
}

Item* SummonScroll(int i)
{
    return s_scrolls[i].create();
}

/* Load scroll information from file.  Example input:
S_CONFUSE  monster_confusion   8   140
S_MAP      magic_mapping       5   150
S_HOLD     hold_monster        3   180
S_SLEEP    sleep               5     5
S_ARMOR    enchant_armor       8   160
S_IDENT    identify           27   100
S_SCARE    scare_monster       4   200
S_GFIND    food_detection      4    50
S_TELEP    teleportation       7   165
S_ENCH     enchant_weapon     10   150
S_CREATE   create_monster      5    75
S_REMOVE   remove_curse        8   105
S_AGGR     aggravate_monsters  4    20
S_NOP      blank_paper         1     5
S_VORPAL   vorpalize_weapon    1   300
*/
void LoadScroll(const std::string& line, int* probability)
{
    if (line.empty() || line[0] == '#')
        return;

    std::istringstream ss(line);

    std::string type;
    ss >> type;
    auto i = s_scrolls_type.find(type);
    if (i == s_scrolls_type.end())
        throw std::runtime_error("Unknown type: " + type);

    std::string name;
    int prob, worth;
    ss >> name >> prob >> worth;
    if (!ss)
        throw std::runtime_error("Error reading: " + line);

    std::replace(name.begin(), name.end(), '_', ' ');
    *probability += prob;
    std::string id = GenerateScrollName();

    i->second.setter(name, id, worth);
    vector_entry e = { *probability, i->second.creator, i->second.getter };
    s_scrolls.push_back(e);
}

void LoadScrolls(const std::string & filename)
{
    int probability = 0;

    if (filename.empty())
        throw std::runtime_error("No scroll file provided");

    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Error reading from: " + filename);

    std::string line;
    while (std::getline(file, line)) {
        LoadScroll(line, &probability);
    }

    if (probability != 100)
        throw std::runtime_error("Scroll probabilities not 100");
}

void PrintScrollDiscoveries()
{
    short order[50];
    int maxnum = s_scrolls.size();

    set_order(order, maxnum);
    int num_found = 0;
    for (int i = 0; i < maxnum; i++) {
        auto e = s_scrolls[order[i]];
        if (e.info().is_discovered() || !e.info().guess().empty())
        {
            std::string line = e.info().name();
            add_line("", "A scroll of %s", line.c_str());
            num_found++;
        }
    }
    if (num_found == 0)
        add_line("", nothing(SCROLL), 0);
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

std::string Scroll::Name() const
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

    auto scroll_info = Info();

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

ItemCategory* Scroll::Category() const
{
    return &Info();
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
    auto scroll_info = Info();

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

void Teleportation::Read()
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
    weapon->vorpalize();
}

ItemCategory MonsterConfusion::info;
ItemCategory MagicMapping::info;
ItemCategory HoldMonster::info;
ItemCategory Sleep::info;
ItemCategory EnchantArmor::info;
ItemCategory Identify::info;
ItemCategory ScareMonster::info;
ItemCategory FoodDetection::info;
ItemCategory Teleportation::info;
ItemCategory EnchantWeapon::info;
ItemCategory CreateMonster::info;
ItemCategory RemoveCurse::info;
ItemCategory AggravateMonsters::info;
ItemCategory BlankPaper::info;
ItemCategory VorpalizeWeapon::info;
