//Contains functions for dealing with things like potions, scrolls, and other items.
//things.c     1.4 (AI Design) 12/14/84
#include <sstream>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#include "random.h"
#include "game_state.h"
#include "things.h"
#include "pack.h"
#include "list.h"
#include "sticks.h"
#include "io.h"
#include "misc.h"
#include "daemons.h"
#include "rings.h"
#include "scrolls.h"
#include "potions.h"
#include "weapons.h"
#include "output_shim.h"
#include "main.h"
#include "armor.h"
#include "daemon.h"
#include "level.h"
#include "food.h"
#include "hero.h"

template<typename T>
Item* createInstance() { return new T; }

template<typename T>
void setInfo(const std::string& name, const std::string& identifier, int worth, const std::string& type) {
    T::info.name(name);
    T::info.identifier(identifier);
    T::info.worth(worth);
    T::info.kind(type);
}

template<typename T>
ItemCategory& getName()
{
    return T::info;
}

struct map_entry
{
    Item*(*creator)();
    void(*setter)(const std::string&, const std::string&, int, const std::string&);
    ItemCategory&(*getter)();
};

struct vector_entry
{
    int prob;
    Item*(*create)();
    ItemCategory&(*info)();
};

struct ItemFactory
{
    void LoadItems(const std::string & filename);

    Item* Create();
    Item* Summon(int i);
    int NumTypes();

    //todo: feels wrong here
    void PrintDiscoveries(const std::string& prefix, int type);

protected:
    std::map<std::string, map_entry> m_types;

private:
    void LoadItem(const std::string& line, int* probability);

    virtual std::string GetIdentifier(int* worth) = 0;
    virtual std::string GetKind() { return std::string(); }

    std::vector<vector_entry> m_items;
};

int ItemFactory::NumTypes()
{
    return m_items.size();
}

Item* ItemFactory::Create()
{
    int r = rnd(100);
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (r < m_items[i].prob) {
            return m_items[i].create();
        }
    }
    throw std::runtime_error("Failed to create item");
}

Item* ItemFactory::Summon(int i)
{
    return m_items[i].create();
}

void ItemFactory::PrintDiscoveries(const std::string& prefix, int type)
{
    short order[50];
    int maxnum = m_items.size();

    set_order(order, maxnum);
    int num_found = 0;
    for (int i = 0; i < maxnum; i++) {
        auto e = m_items[order[i]];

        std::string line;
        if (e.info().is_discovered())
            line = prefix + " of " + e.info().name();
        else if (!e.info().guess().empty())
            line = prefix + " called " + e.info().guess();

        if (!line.empty())
        {
            add_line("", "%s", line.c_str());
            num_found++;
        }
    }
    if (num_found == 0)
        add_line("", nothing(type), 0);
}


void ItemFactory::LoadItem(const std::string& line, int* probability)
{
    if (line.empty() || line[0] == '#' || line[0] == ';')
        return;

    std::istringstream ss(line);

    std::string type;
    ss >> type;
    auto i = m_types.find(type);
    if (i == m_types.end())
        throw std::runtime_error("Unknown type: " + type);

    std::string name;
    int prob, worth;
    ss >> name >> prob >> worth;
    if (!ss)
        throw std::runtime_error("Error reading: " + line);

    std::replace(name.begin(), name.end(), '_', ' ');
    *probability += prob;
    std::string id = GetIdentifier(&worth);
    std::string kind = GetKind();

    i->second.setter(name, id, worth, kind);
    vector_entry e = { *probability, i->second.creator, i->second.getter };
    m_items.push_back(e);
}

void ItemFactory::LoadItems(const std::string & filename)
{
    int probability = 0;

    if (filename.empty())
        throw std::runtime_error("No scroll file provided");

    std::ifstream file(filename);
    if (!file)
        throw std::runtime_error("Error opening: " + filename);

    std::string line;
    while (std::getline(file, line)) {
        LoadItem(line, &probability);
    }

    if (probability != 100)
        throw std::runtime_error("Scroll probabilities not 100");
}

#define ITEM_MAP_ENTRY(Type) {&createInstance<Type>, &setInfo<Type>, &getName<Type>}

struct ScrollFactory : public ItemFactory
{
    ScrollFactory()
    {
        m_types = {
            { "S_CONFUSE", ITEM_MAP_ENTRY(MonsterConfusion) },
            { "S_MAP", ITEM_MAP_ENTRY(MagicMapping) },
            { "S_HOLD", ITEM_MAP_ENTRY(HoldMonster) },
            { "S_SLEEP", ITEM_MAP_ENTRY(Sleep) },
            { "S_ARMOR", ITEM_MAP_ENTRY(EnchantArmor) },
            { "S_IDENT", ITEM_MAP_ENTRY(Identify) },
            { "S_SCARE", ITEM_MAP_ENTRY(ScareMonster) },
            { "S_GFIND", ITEM_MAP_ENTRY(FoodDetection) },
            { "S_TELEP", ITEM_MAP_ENTRY(TeleportationScroll) },
            { "S_ENCH", ITEM_MAP_ENTRY(EnchantWeapon) },
            { "S_CREATE", ITEM_MAP_ENTRY(CreateMonster) },
            { "S_REMOVE", ITEM_MAP_ENTRY(RemoveCurse) },
            { "S_AGGR", ITEM_MAP_ENTRY(AggravateMonsters) },
            { "S_NOP", ITEM_MAP_ENTRY(BlankPaper) },
            { "S_VORPAL", ITEM_MAP_ENTRY(VorpalizeWeapon) },
        };
    }

private:
    virtual std::string GetIdentifier(int* worth) {
        return GenerateScrollName();
    }
};
    
ScrollFactory s_scrolls;

void LoadScrolls(const std::string & filename)
{
    s_scrolls.LoadItems(filename);
}

void PrintScrollDiscoveries()
{
    s_scrolls.PrintDiscoveries("A scroll", SCROLL);
}

Item* CreateScroll()
{
    return s_scrolls.Create();
}

Item* SummonScroll(int i)
{
    return s_scrolls.Summon(i);
}

int NumScrollTypes()
{
    return s_scrolls.NumTypes();
}

static char *rainbow[] =
{
    "amber",
    "aquamarine",
    "black",
    "blue",
    "brown",
    "clear",
    "crimson",
    "cyan",
    "ecru",
    "gold",
    "green",
    "grey",
    "magenta",
    "orange",
    "pink",
    "plaid",
    "purple",
    "red",
    "silver",
    "tan",
    "tangerine",
    "topaz",
    "turquoise",
    "vermilion",
    "violet",
    "white",
    "yellow"
};

#define NCOLORS (sizeof(rainbow)/sizeof(char *))

struct PotionFactory : public ItemFactory
{
    PotionFactory()
    {
        m_types = {
            { "P_CONFUSE", ITEM_MAP_ENTRY(Confusion) },
            { "P_PARALYZE", ITEM_MAP_ENTRY(Paralysis) },
            { "P_POISON", ITEM_MAP_ENTRY(Poison) },
            { "P_STRENGTH", ITEM_MAP_ENTRY(GainStrength) },
            { "P_SEEINVIS", ITEM_MAP_ENTRY(SeeInvisiblePotion) },
            { "P_HEALING", ITEM_MAP_ENTRY(Healing) },
            { "P_MFIND", ITEM_MAP_ENTRY(MonsterDetection) },
            { "P_TFIND", ITEM_MAP_ENTRY(MagicDetection) },
            { "P_RAISE", ITEM_MAP_ENTRY(RaiseLevel) },
            { "P_XHEAL", ITEM_MAP_ENTRY(ExtraHealing) },
            { "P_HASTE", ITEM_MAP_ENTRY(HasteSelf) },
            { "P_RESTORE", ITEM_MAP_ENTRY(RestoreStrength) },
            { "P_BLIND", ITEM_MAP_ENTRY(Blindness) },
            { "P_NOP", ITEM_MAP_ENTRY(ThirstQuenching) },
        };

        for (int i = 0; i < NCOLORS; i++) 
            used[i] = false;
    }

private:
    virtual std::string GetIdentifier(int* worth)
    {
        int j;
        do {
            j = rnd(NCOLORS);
        }
        while (used[j]);

        used[j] = true;
        return rainbow[j];
    }

    bool used[NCOLORS];
};

PotionFactory s_potions;

void LoadPotions(const std::string & filename)
{
    s_potions.LoadItems(filename);
}

void PrintPotionDiscoveries()
{
    s_potions.PrintDiscoveries("A potion", POTION);
}

Item * CreatePotion()
{
    return s_potions.Create();
}

Item * SummonPotion(int i)
{
    return s_potions.Summon(i);
}

int NumPotionTypes()
{
    return s_potions.NumTypes();
}

static char *wood[] =
{
    "avocado wood",
    "balsa",
    "bamboo",
    "banyan",
    "birch",
    "cedar",
    "cherry",
    "cinnibar",
    "cypress",
    "dogwood",
    "driftwood",
    "ebony",
    "elm",
    "eucalyptus",
    "fall",
    "hemlock",
    "holly",
    "ironwood",
    "kukui wood",
    "mahogany",
    "manzanita",
    "maple",
    "oaken",
    "persimmon wood",
    "pecan",
    "pine",
    "poplar",
    "redwood",
    "rosewood",
    "spruce",
    "teak",
    "walnut",
    "zebrawood"
};

#define NWOOD (sizeof(wood)/sizeof(char *))

static char *metal[] =
{
    "aluminum",
    "beryllium",
    "bone",
    "brass",
    "bronze",
    "copper",
    "electrum",
    "gold",
    "iron",
    "lead",
    "magnesium",
    "mercury",
    "nickel",
    "pewter",
    "platinum",
    "steel",
    "silver",
    "silicon",
    "tin",
    "titanium",
    "tungsten",
    "zinc"
};

#define NMETAL (sizeof(metal)/sizeof(char *))


struct StickFactory : public ItemFactory
{
    StickFactory()
    {
        m_types = {
            { "WS_LIGHT", ITEM_MAP_ENTRY(Light) },
            { "WS_HIT", ITEM_MAP_ENTRY(Striking) },
            { "WS_ELECT", ITEM_MAP_ENTRY(Lightning) },
            { "WS_FIRE", ITEM_MAP_ENTRY(Fire) },
            { "WS_COLD", ITEM_MAP_ENTRY(Cold) },
            { "WS_POLYMORPH", ITEM_MAP_ENTRY(Polymorph) },
            { "WS_MISSILE", ITEM_MAP_ENTRY(MagicMissileStick) },
            { "WS_HASTE_M", ITEM_MAP_ENTRY(HasteMonster) },
            { "WS_SLOW_M", ITEM_MAP_ENTRY(SlowMonster) },
            { "WS_DRAIN", ITEM_MAP_ENTRY(DrainLife) },
            { "WS_NOP", ITEM_MAP_ENTRY(Nothing) },
            { "WS_TELAWAY", ITEM_MAP_ENTRY(TeleportAway) },
            { "WS_TELTO", ITEM_MAP_ENTRY(TeleportTo) },
            { "WS_CANCEL", ITEM_MAP_ENTRY(Cancellation) },
        };

        for (int i = 0; i < NWOOD; i++)
            woodused[i] = false;
        for (int i = 0; i < NMETAL; i++)
            metused[i] = false;
    }

private:
    bool metused[NMETAL];
    bool woodused[NWOOD];
    std::string type;

    virtual std::string GetIdentifier(int* worth)
    {
        const char *str;

        for (;;) if (rnd(2) == 0)
        {
            int j = rnd(NMETAL);
            if (!metused[j]) {
                type = "wand";
                str = metal[j];
                metused[j] = true;
                break;
            }
        }
        else
        {
            int j = rnd(NWOOD);
            if (!woodused[j]) {
                type = "staff";
                str = wood[j];
                woodused[j] = true;
                break;
            }
        }
        return str;
    }
    
    virtual std::string GetKind() override
    {
        return type;
    }
};

StickFactory s_sticks;

void LoadSticks(const std::string & filename)
{
    s_sticks.LoadItems(filename);
}

void PrintStickDiscoveries()
{
    s_sticks.PrintDiscoveries("A stick", STICK);
}

Item* CreateStick()
{
    return s_sticks.Create();
}

Item* SummonStick(int i)
{
    return s_sticks.Summon(i);
}

int NumStickTypes()
{
    return s_sticks.NumTypes();
}

typedef struct { char *st_name; int st_value; } STONE;
static STONE stones[] =
{
    { "agate",           25 },
    { "alexandrite",     40 },
    { "amethyst",        50 },
    { "carnelian",       40 },
    { "diamond",        300 },
    { "emerald",        300 },
    { "germanium",      225 },
    { "granite",          5 },
    { "garnet",          50 },
    { "jade",           150 },
    { "kryptonite",     300 },
    { "lapis lazuli",    50 },
    { "moonstone",       50 },
    { "obsidian",        15 },
    { "onyx",            60 },
    { "opal",           200 },
    { "pearl",          220 },
    { "peridot",         63 },
    { "ruby",           350 },
    { "sapphire",       285 },
    { "stibotantalite", 200 },
    { "tiger eye",       50 },
    { "topaz",           60 },
    { "turquoise",       70 },
    { "taaffeite",      300 },
    { "zircon",          80 }
};

#define NSTONES (sizeof(stones)/sizeof(STONE))

struct RingFactory : public ItemFactory
{
    RingFactory()
    {
        m_types = {
            { "R_PROTECT", ITEM_MAP_ENTRY(Protection) },
            { "R_ADDSTR", ITEM_MAP_ENTRY(AddStrength) },
            { "R_SUSTSTR", ITEM_MAP_ENTRY(SustainStrength) },
            { "R_SEARCH", ITEM_MAP_ENTRY(Searching) },
            { "R_SEEINVIS", ITEM_MAP_ENTRY(SeeInvisibleRing) },
            { "R_NOP", ITEM_MAP_ENTRY(Adornment) },
            { "R_AGGR", ITEM_MAP_ENTRY(AggravateMonster) },
            { "R_ADDHIT", ITEM_MAP_ENTRY(Dexterity) },
            { "R_ADDDAM", ITEM_MAP_ENTRY(IncreaseDamage) },
            { "R_REGEN", ITEM_MAP_ENTRY(Regeneration) },
            { "R_DIGEST", ITEM_MAP_ENTRY(SlowDigestion) },
            { "R_TELEPORT", ITEM_MAP_ENTRY(TeleportationRing) },
            { "R_STEALTH", ITEM_MAP_ENTRY(Stealth) },
            { "R_SUSTARM", ITEM_MAP_ENTRY(MaintainArmor) },
        };
        for (int i = 0; i < NSTONES; i++) 
            used[i] = false;
    }

private:
    bool used[NSTONES];

    virtual std::string GetIdentifier(int* worth)
    {
        int j;
        do { 
            j = rnd(NSTONES);
        } while (used[j]);

        used[j] = true;
        *worth += stones[j].st_value;
        return stones[j].st_name;
    }
};

RingFactory s_rings;

void LoadRings(const std::string & filename)
{
    s_rings.LoadItems(filename);
}

void PrintRingDiscoveries()
{
    s_rings.PrintDiscoveries("A ring", RING);
}

Item* CreateRing()
{
    return s_rings.Create();
}

Item* SummonRing(int i)
{
    return s_rings.Summon(i);
}

int NumRingTypes()
{
    return s_rings.NumTypes();
}

struct MagicItem things[NUMTHINGS] =
{
  {0, 27 }, //potion
  {0, 30 }, //scroll
  {0, 17 }, //food
  {0,  8 }, //weapon
  {0,  8 }, //armor
  {0,  5 }, //ring
  {0,  5 }  //stick
};

static short order[50]; //todo:eliminate static
static int line_cnt = 0; //todo:eliminate static

//init_things: Initialize the probabilities for types of things
void init_things()
{
    struct MagicItem *mp;
    for (mp = &things[1]; mp <= &things[NUMTHINGS - 1]; mp++)
        mp->prob += (mp - 1)->prob;
}

void Item::discover()
{
    set_known();
    if (Category()) {
        Category()->discover();
    }
}

void Item::call_it()
{
    if (Category()) {
        char buf[MAXNAME];
        if (Category()->is_discovered())
            Category()->guess("");
        else if (Category()->guess().empty())
        {
            msg("%scall it? ", noterse("what do you want to "));
            getinfo(buf, MAXNAME);
            if (*buf != ESCAPE)
                Category()->guess(buf);
            msg("");
        }
    }
}

void Item::set_as_target_of(Monster * m)
{
    m->set_destination(&m_position);
}

void chopmsg(char *s, char *shmsg, char *lnmsg, ...)
{
    va_list argptr;
    va_start(argptr, lnmsg);
    vsprintf(s, short_msgs() ? shmsg : lnmsg, argptr);
    va_end(argptr);
}

//do_drop: Put something down
bool do_drop()
{
    byte ch;
    Item *nobj, *op;

    ch = game->level().get_tile(game->hero().position());
    if (ch != FLOOR && ch != PASSAGE) {
        //mdk: trying to drop item into occupied space counts as turn
        msg("there is something there already");
        return true;
    }

    if ((op = get_item("drop", 0)) == NULL) 
        return false;

    if (!can_drop(op, true)) 
        return true;

    //Take it out of the pack
    if (op->m_count >= 2 && op->m_type != WEAPON)
    {
        op->m_count--;
        nobj = op->Clone();
        nobj->m_count = 1;
        op = nobj;
    }
    else
        game->hero().m_pack.remove(op);
    //Link it into the level object list
    game->level().items.push_front(op);
    op->set_position(game->hero().position());
    game->level().set_tile(op->position(), op->m_type);
    msg("dropped %s", op->inventory_name(game->hero(), true).c_str());

    return true;
}

//can_drop: Do special checks for dropping or unweilding|unwearing|unringing
bool can_drop(Item *op, bool unequip)
{
    if (op == NULL)
        return true;
    if (op != game->hero().get_current_armor() && op != game->hero().get_current_weapon() &&
        op != game->hero().get_ring(LEFT) && op != game->hero().get_ring(RIGHT))
        return true;
    if (op->is_cursed()) {
        msg("you can't.  It appears to be cursed");
        return false;
    }
    if (!unequip)
        return true;

    if (op == game->hero().get_current_weapon()) {
        game->hero().set_current_weapon(NULL);
        return true;
    }

    else if (op == game->hero().get_current_armor()) {
        waste_time();  //mdk: taking off/dropping/throwing current armor takes two turns
        game->hero().set_current_armor(NULL);
        return true;
    }

    int hand;

    if (op != game->hero().get_ring(hand = LEFT)) {
        if (op != game->hero().get_ring(hand = RIGHT))
        {
            debug("Candrop called with funny thing");
            return true;
        }
    }
    Ring* ring = game->hero().get_ring(hand);
    game->hero().set_ring(hand, NULL);
    ring->Remove();

    return true;
}

//new_thing: Return a new thing
Item* Item::CreateItem()
{
    //Decide what kind of object it will be. If we haven't had food for a while, let it be food.
    switch (game->no_food > 3 ? 2 : pick_one(things, NUMTHINGS))
    {
    case 0:
        return CreatePotion();
        break;

    case 1:
        return CreateScroll();
        break;

    case 2:
        return CreateFood();
        break;

    case 3:
        return create_weapon();
        break;

    case 4:
        return create_armor();
        break;

    case 5:
        return CreateRing();
        break;

    case 6:
        return CreateStick();
        break;

    default:
        debug("Picked a bad kind of object");
        wait_for(' ');
        break;
    }
    return 0;
}

//todo: remove this wrapper
int pick_one(std::vector<MagicItem> magic)
{
    MagicItem* a = new MagicItem[magic.size()];
    for (size_t i = 0; i < magic.size(); ++i)
        a[i] = magic[i];
    int r = pick_one(a, magic.size());
    delete[] a;
    return r;
}

//pick_one: Pick an item out of a list of nitems possible magic items
int pick_one(struct MagicItem *magic, int nitems)
{
    struct MagicItem *end;
    int i;
    struct MagicItem *start;

    start = magic;
    for (end = &magic[nitems], i = rnd(100); magic < end; magic++) if (i < magic->prob) break;
    if (magic == end)
    {
        debug("bad pick_one: %d from %d items", i, nitems);
        for (magic = start; magic < end; magic++)
            debug("%s: %d%%", magic->name, magic->prob);

        magic = start;
    }
    return magic - start;
}

//do_discovered: list what the player has discovered in this game of a certain type
bool do_discovered()
{
    if (game->wizard().enabled()) {
        debug_screen();
        return false;
    }

    print_disc(POTION);
    add_line("", " ", 0);
    print_disc(SCROLL);
    add_line("", " ", 0);
    print_disc(RING);
    add_line("", " ", 0);
    print_disc(STICK);
    end_line("");

    return false;
}

//print_disc: Print what we've discovered of type 'type'
void print_disc(byte type)
{
    if (type == SCROLL) {
        PrintScrollDiscoveries();
        return;
    }
    else if (type == POTION) {
        PrintPotionDiscoveries();
        return;
    }
    else if (type == STICK) {
        PrintStickDiscoveries();
        return;
    }
    else if (type == RING) {
        PrintRingDiscoveries();
    }
}

//set_order: Set up order for list
void set_order(short *order, int numthings)
{
    int i, r, t;

    for (i = 0; i < numthings; i++) order[i] = i;
    for (i = numthings; i > 0; i--)
    {
        r = rnd(i);
        t = order[i - 1];
        order[i - 1] = order[r];
        order[r] = t;
    }
}

//add_line: Add a line to the list of discoveries
int add_line(const char *use, const char *fmt, const char *arg)
{
    int x, y;
    int retchar = ' ';
    const int LINES = game->screen().lines();

    if (line_cnt == 0) {
        game->screen().wdump();
        game->screen().clear();
    }
    if (line_cnt >= LINES - 1 || fmt == NULL)
    {
        game->screen().move(LINES - 1, 0);
        if (*use)
            game->screen().printw("-Select item to %s. Esc to cancel-", use);
        else
            game->screen().addstr("-Press space to continue-");
        do retchar = readchar(); while (retchar != ESCAPE && retchar != ' ' && (!islower(retchar)));
        game->screen().clear();
        line_cnt = 0;
    }
    if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0'))
    {
        game->screen().move(line_cnt, 0);
        game->screen().printw(fmt, arg);
        game->screen().getrc(&x, &y);
        //if the line wrapped but nothing was printed on this line you might as well use it for the next item
        if (y != 0) line_cnt = x + 1;
    }
    return (retchar);
}

//end_line: End the list of lines
int end_line(const char *use)
{
    int retchar;

    retchar = add_line(use, 0, 0);
    game->screen().wrestor();
    line_cnt = 0;
    return (retchar);
}

//nothing: Set up prbuf so that message for "nothing found" is there
char *nothing(byte type)
{
    char *sp, *tystr;

    sprintf(prbuf, "Haven't discovered anything");
    if (in_small_screen_mode()) sprintf(prbuf, "Nothing");
    sp = &prbuf[strlen(prbuf)];
    switch (type)
    {
    case POTION: tystr = "potion"; break;
    case SCROLL: tystr = "scroll"; break;
    case RING: tystr = "ring"; break;
    case STICK: tystr = "stick"; break;
    }
    sprintf(sp, " about any %ss", tystr);
    return prbuf;
}
