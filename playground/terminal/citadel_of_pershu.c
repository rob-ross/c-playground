// citadel_of_pershu.c
//
// ported by Rob Ross
// from a BASIC text adventure by Tim Hartnell, 1983
//
//
// Created 2026/05/15 01:35:01 PDT

// make :
// cd /Users/robross/Documents/Development/CLionProjects/CS50x/playground/terminal
//  DEBUG:
/*
clang -fsanitize=address -fsanitize=leak -Wall -Werror \
    -Wno-unused-const-variable -Wno-unused-variable -Wno-unused-function \
    -std=c23 -o citadel_of_pershu.out citadel_of_pershu.c mersenne_twister.c

  PROD:
 clang -std=c23 -o citadel_of_pershu.out citadel_of_pershu.c mersenne_twister.c

*/

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <poll.h>
#endif
#include <sys/_types/_useconds_t.h>

#include "mersenne_twister.h"


constexpr int NUM_ROOMS        = 48;
constexpr int NUM_DEATH_ROOMS  =  4;

constexpr int START_ROOM       =  6;
constexpr int END_ROOM         = 31;
constexpr int LIBRARY_ROOM     = 4;
constexpr int WINE_CELLAR_EAST = 12;
constexpr int BEDCHAMBER_ROOM  = 15;
constexpr int MARBLE_HALL      = 20;
constexpr int GLOVE_STOREROOM  = 21;
constexpr int SILVER_CROSSES_STOREROOM = 22;
constexpr int DROWNING_ROOM    = 44;

constexpr int NUM_TREASURES  = 19;
constexpr int NUM_MONSTERS   = 20;

enum Direction {
    DIRECTION_ERR = -1,
    DIRECTION_NORTH = 0,
    DIRECTION_SOUTH,
    DIRECTION_EAST,
    DIRECTION_WEST,
    DIRECTION_UP,
    DIRECTION_DOWN,
};

// direction in "NSEWUD"
enum Direction calc_direction_index(char const direction_char) {
    switch (toupper(direction_char)) {
        case 'N': return DIRECTION_NORTH;
        case 'S': return DIRECTION_SOUTH;
        case 'E': return DIRECTION_EAST;
        case 'W': return DIRECTION_WEST;
        case 'U': return DIRECTION_UP;
        case 'D': return DIRECTION_DOWN;
        default: return DIRECTION_ERR;
    }
}

const char * direction_string(const enum Direction direction_index) {
    switch (direction_index) {
        case DIRECTION_ERR:   return "ERROR";
        case DIRECTION_NORTH: return "NORTH";
        case DIRECTION_SOUTH: return "SOUTH";
        case DIRECTION_EAST:  return "EAST";
        case DIRECTION_WEST:  return "WEST";
        case DIRECTION_UP:    return "UP";
        case DIRECTION_DOWN:  return "DOWN";
        default:              return "UNKNOWN";
    }
}

char const * const BAD_MOVE_DESC[6] = {
    "NO EXIT THAT WAY",
    "THERE IS NO EXIT SOUTH",
    "YOU CANNOT GO IN THAT DIRECTION",
    "IN THAT WAY LIES MADNESS",
    "THERE IS NO WAY UP FROM HERE",
    "YOU CANNOT DESCEND FROM HERE",
};

struct RandomText {
    const char *text;  // displayed if chance_percent is satisifed
    const char *else_text; // if not null, displayed when chance_percent not satisfied
    double chance_percent;  // betwen 0 and 1. Random number between 0 and 1  must be less (<) than this to be displayed
};

struct RandomTextArray {
    size_t length;
    struct RandomText lines[];  // flexible array
};

enum StatIndex {
    STAT_NULL,
    STAT_STRENGTH,
    STAT_CHARISMA,
    STAT_DEXTERITY,
    STAT_INTELLIGENCE,
    STAT_WISDOM,
    STAT_CONSTITUTION,
    STAT_COUNT // Useful for loops and array sizing
};

// Just the raw integer fields
#define CHAR_STATS_LIST   \
    int null_stat;        \
    int strength;         \
    int charisma;         \
    int dexterity;        \
    int intelligence;     \
    int wisdom;           \
    int constitution;

// The union logic that maps the array to the fields
#define CHAR_STATS_UNION_BODY     \
    int as_array[STAT_COUNT];     \
    struct { CHAR_STATS_LIST };

struct CharStats {
    union { CHAR_STATS_UNION_BODY };
};

struct Monster {
    char const * name;
    int monster_index;
    union {
        struct CharStats stats; // Named access: m.stats.strength
        struct { CHAR_STATS_UNION_BODY }; // Anonymous access: m.strength & m.as_array
    };
};

struct Treasure {
    char const * name;
    int treasure_index;
    int value;
};

struct Room {
    int id;
    char const * name;
    char const * desc;
    struct RandomTextArray  * preamble;
    struct RandomTextArray  * epilog;
    struct Monster  monster;
    struct Treasure treasure;
};


// todo (rob) We should just store the full description on a single line of text (no embedded newlines), and let the
// display argument paginate the text as needed for the display. Currently we use the same line breaks as in the original
// BASIC app.
static struct Room ROOMS[NUM_ROOMS] = {
{.id =  0,  .name= "NULL ROOM",  .desc = "NULL ROOM"},
{.id =  1,  .name= "ROOM 1",  .desc = "An underground river flows swiftly by."},
{.id =  2,  .name= "ROOM 2",  .desc = "You are in the Citadel's food storage area.\nOld cheeses and black loaves of bread can\nbe seen, as well as many sacks of supplies."},
{.id =  3,  .name= "ROOM 3",  .desc = "You are in the Citadel's kitchen. A Huge\njoint of meat turns slowly over a raging\nfire. Doors lead into cupboards, as well\nas to the west and to the south."},
{.id =  4,  .name= "ROOM 4",  .desc = "This is the Central Library. Leather-bound\nvolumes line the walls, right up to the\nornately carved ceiling."},
{.id =  5,  .name= "ROOM 5",  .desc = "This room is an awful mess. It used to be\nan artist's studio. Paint and old\neasels lie around the floor."},
{.id =  6,  .name= "ROOM 6",  .desc = "This is the entrance to the Citadel of Pershu.\nTurn now, if you wish. Many stronger than you\nhave taken fright at its menacing towers and\ndark portals. If you wish to proceed, move\neast towards the black gaping doorway."},
{.id =  7,  .name= "ROOM 7",  .desc = "A stone altar stands in the middle of the room\nwith two dead candles on it. An old book lies\non one part of the altar top, and a faded red\nparchment cloth covers the front of it."},
{.id =  8,  .name= "ROOM 8",  .desc = "You stand high on the black tower, the\nCitadel stretches to the north, south\nand east of you.\nThere is only one way out."},
{.id =  9,  .name= "ROOM 9",  .desc = "You are in the northern section of the\nCitadel's large wine cellar. Heavy\nbarrels lie all around you in this end\nof the cellar. There is a door to the north\nand one to the south."},
{.id = 10,  .name= "ROOM 10", .desc = "You are in the west wing of the wine\ncellar. There is a door to the west and\none to the east. The central circular\npart of the cellar lies beyond the\neast door."},
{.id = 11,  .name= "ROOM 11", .desc = "You are in the central circular\narea of the wine cellar. There is\na door at each compass point."},
{.id = 12,  .name= "ROOM 12", .desc = "You are in the east section of the\nwine cellar. There is a door to the\nwest and one - which you cannot use,\nas it only allows entrance to where\nyou now stand - to the east."},
{.id = 13,  .name= "ROOM 13", .desc = "There are many, many wine bottles here\nlying on their sides in this southern\nsection of the wine cellar. There is a\ndark, unfriendly-looking hole to the west\nand doors to the north and to the south."},
{.id = 14,  .name= "ROOM 14", .desc = "This is the Citadel's armory. Row upon row\nof shiny suits of armor are stored here." },
{.id = 15,  .name= "ROOM 15", .desc = "You are in the ruler's bedchamber.\nA large fire burns in the south of\nthe room, with a small door beside\nit. Other exits are to the north\nand to the west." },
{.id = 16,  .name= "ROOM 16", .desc = "Sand covers the floor of this curious\nroom, heaped into drifts.\nBy peeping over the 'dunes' you can\nsee a golden passage way leads to the\nwest, and there is a door to the south. You are not sure whether or not you\nhave seen all the exits." },
{.id = 17,  .name= "ROOM 17", .desc = "You are in the picture gallery. Portraits\nof long-dead princes line all of the\nwalls. The room is dominated by a huge\nlandsape, hanging above the exit to the\neast which leads, via the gold passage way\nback to that curious room of sand." },
{.id = 18,  .name= "ROOM 18", .desc = "You are on a remote tower balcony.\nThere are stairs here." },
{.id = 19,  .name= "ROOM 19", .desc = "You wallk beneath a stone archway.\nYou can only walk north or south\nunless you decide to take the stairs." },
{.id = 20,  .name= "ROOM 20", .desc = "This vast hall has a marble floor, and\nthe slightest sound echos violently.\nThere are purple drapes concealing\nthe exits from this hall." },
{.id = 21,  .name= "ROOM 21", .desc = "You are in the glove storeroom.\nThe west door radiates heat.\nAnother door leads to the south." },
{.id = 22,  .name= "ROOM 22", .desc = "You are in the silver crosses storeroom.\nThere are only two exits." },
{.id = 23,  .name= "ROOM 23", .desc = "You are in the amulet storeroom.\nDoors lead north and south." },
{.id = 24,  .name= "ROOM 24", .desc = "You are in the kazoo storeroom.\nThere are two exits." },
{.id = 25,  .name= "ROOM 25", .desc = "You are in the satchel storeroom." },
{.id = 26,  .name= "ROOM 26", .desc = "You are in the storeroom for wooden\nboxes... There are two exits." },
{.id = 27,  .name= "ROOM 27", .desc = "This is where printed vases are\nstored... As you can easily see." },
{.id = 28,  .name= "ROOM 28", .desc = "The heavy air of this area seems to make\nyour torch very dim. You can hardly see\nthat air is rushing up from somewhere.\nYou can just make out that this area must\nbe a mine of some sort." },
{.id = 29,  .name= "ROOM 29", .desc = "You appear to be in an endless labyrinth,\nlined with paintings.........\nWhichever way you turn, there seems to be\nmore tunnels, all lined with paintings." },
{.id = 30,  .name= "ROOM 30", .desc = "This is the southern tower of the Citadel." },
{.id = 31,  .name= "ROOM 31", .desc = "Well done, you have managed to find the exit.\nTake a deep breath of good, clean air..........." },
{.id = 32,  .name= "ROOM 32", .desc = "This room is filled with swirling smoke,\nso you cannot see... Air rushes past a\nstatue of the goddess Diana. This\nmust be the Citadel's meditation chamber." },
{.id = 33,  .name= "ROOM 33", .desc = "A small forked bridge crosses a stream here.\nYou can move north, south, or west." },
{.id = 34,  .name= "ROOM 34", .desc = "You are in a rough stone cavern. Stairs\nlead up from here.\nThere is also a single door which\nleads away from the cavern." },
{.id = 35,  .name= "ROOM 35", .desc = "This is the former Citadel undergound\nstable. It smells terrible." },
{.id = 36,  .name= "ROOM 36", .desc = "You find yourself in an underground\ncourtyard. Strange, twisted trees are\naround you, and a wind of incredible\ncoldness blows from the east." },
{.id = 37,  .name= "ROOM 37", .desc = "This is the Oracle Room, although the\nmystic voice has not spoken for many\nyears." },
{.id = 38,  .name= "ROOM 38", .desc = "Horrors. A cold shudder passes through you as you\nrealize this is the priests' sacrifice room.\nDried-up blood is on the floor and a\nskull grins at you from high on the wall." },
{.id = 39,  .name= "ROOM 39", .desc = "Old straw mattresses and rings chained to the\nwall tell you this was the Citadel's dungeon. The dungeon seems to stretch forever, with many\nsmall partitioned areas...." },
{.id = 40,  .name= "ROOM 40", .desc = "You are in a small alcove, with a solid\ngray granite throne in the middle of it." },
{.id = 41,  .name= "ROOM 41", .desc = "This is the orc's guardroom, way below\nthe ground. A stairwell ends here and\na door leads to the east." },
{.id = 42,  .name= "ROOM 42", .desc = "There is a healing pool here, with a\ndangerous, swirling area of water." },
{.id = 43,  .name= "ROOM 43", .desc = "The Underpriests of Odric used this\ntiny hall for their forbidden worship\neons ago. It is an unpleasant area,\nso you are thrilled to see a set of\nstone stairs." },

{.id = 44,  .name= "DEATH BY DROWING", .desc = "Water covers your head.\nYou are drowning.\nGLUG... GASP............" },
{.id = 45,  .name= "DEATH BY BURNING", .desc = "The flames strike at you...\nas you slowly burn to death..."},
{.id = 46,  .name= "DEATH BY FREEZING", .desc = "You are hit by a freezing spell\nand turn into a block of perpetual\nliving stone. This is the end."},
{.id = 47,  .name= "BOTTOMLESS PIT", .desc = "You tumble down a bottomless pit.\nDown, down, down..."},

};



enum RoomGraphIndex {
    RGINDEX_NORTH, RGINDEX_SOUTH, RGINDEX_EAST, RGINDEX_WEST, RGINDEX_UP, RGINDEX_DOWN,
    RGINDEX_TREASURE, RGINDEX_MONSTER,
    RGINDEX_COUNT
};

static int ROOM_GRAPH[NUM_ROOMS][RGINDEX_COUNT] = {
    {  0,  0,  0,  0,  0,  0,  0,  0 },  //  NULL ROOM 0

    {  1,  4,  1,  8,  0,  0,  0,  0 },  //  ROOM 1
    {  0,  5,  3,  0,  0,  0,  0,  0 },  //  ROOM 2
    {  3,  7,  3,  2,  0,  0,  0,  0 },  //  ROOM 3
    {  1,  0,  5,  0,  0,  0,  2,  0 },  //  ROOM 4
    {  2,  0,  0,  4,  0,  0,  0,  0 },  //  ROOM 5
    {  0,  0,  7,  0,  0,  0,  1,  0 },  //  ROOM 6, ENTRANCE
    {  3, 14, 15,  6,  0,  0,  0,  0 },  //  ROOM 7
    {  1,  8,  8,  8,  0,  0,  0,  0 },  //  ROOM 8
    { 10, 11,  0,  0,  0,  0,  0,  0 },  //  ROOM 9
    {  0,  0, 11,  9,  0,  0,  0,  0 },  //  ROOM 10
    {  9, 13, 12, 10,  0,  0,  0,  0 },  //  ROOM 11
    {  0,  0,  0, 11,  0,  0,  0,  0 },  //  ROOM 12
    { 11, 16,  0, 44,  0,  0,  0,  0 },  //  ROOM 13
    {  7,  0,  0,  0,  0,  0,  0,  0 },  //  ROOM 14
    {  7, 45,  0, 12,  0,  0,  0,  0 },  //  ROOM 15
    {  0, 19,  0, 17,  0, 37,  0,  0 },  //  ROOM 16
    {  0,  0, 16,  0,  0,  0,  0,  0 },  //  ROOM 17
    {  0, 30,  0,  0,  0, 34,  0,  0 },  //  ROOM 18
    { 16, 28,  0,  0,  0, 43,  0,  0 },  //  ROOM 19
    {  0, 31, 22,  0,  0,  0,  0,  0 },  //  ROOM 20
    {  0, 23,  0, 45,  0,  0,  3,  0 },  //  ROOM 21
    {  0, 24,  0, 20,  0,  0,  0,  0 },  //  ROOM 22
    { 21, 25,  0,  0,  0,  0,  0,  0 },  //  ROOM 23
    { 22,  0, 25,  0,  0,  0,  0,  0 },  //  ROOM 24
    { 23, 27, 30, 24,  0,  0,  0,  0 },  //  ROOM 25
    {  0, 29, 27,  0,  0,  0,  0,  0 },  //  ROOM 26
    { 25,  0,  0, 26,  0,  0,  0,  0 },  //  ROOM 27
    { 19, 28, 28, 28,  0, 47,  0,  0 },  //  ROOM 28
    { 26, 29, 29, 29,  0,  0,  0,  0 },  //  ROOM 29
    { 18,  0,  0, 25,  0,  0,  0,  0 },  //  ROOM 30
    { 20,  0,  0,  0,  0,  0,  0,  0 },  //  ROOM 31, END ROOM
    {  0,  0, 34,  0,  0, 47,  0,  0 },  //  ROOM 32
    { 34, 36,  0, 35,  0,  0,  0,  0 },  //  ROOM 33
    { 34, 33, 34, 32, 18,  0,  0,  0 },  //  ROOM 34
    { 33, 38, 36,  0,  0,  0,  0,  0 },  //  ROOM 35
    { 33, 39, 46, 35,  0,  0,  0,  0 },  //  ROOM 36
    {  0, 40,  0,  0, 16,  0,  0,  0 },  //  ROOM 37
    { 35,  0,  0,  0,  0, 41,  0,  0 },  //  ROOM 38
    { 36, 39, 40, 39,  0,  0,  0,  0 },  //  ROOM 39
    { 37,  0,  0, 39,  0,  0,  0,  0 },  //  ROOM 40
    {  0,  0, 42,  0, 38,  0,  0,  0 },  //  ROOM 41
    { 42, 43, 42, 41,  0, 47,  0,  0 },  //  ROOM 42
    {  0,  0, 42,  0, 19,  0,  0,  0 },  //  ROOM 43

        // Death rooms
    {  0,  0,  0,  0,  0,  0,  0,  0 },  //  DEATH BY DROWING
    {  0,  0,  0,  0,  0,  0,  0,  0 },  //  DEATH BY BURNING
    {  0,  0,  0,  0,  0,  0,  0,  0 },  //  DEATH BY FREEZING
    {  0,  0,  0,  0,  0,  0,  0,  0 },  //  BOTTOMLESS PIT

};

// first 9 elements are items the user can use, carry, or drop (and pick up again.)
// From Emeralds and higher, these are treasure that are converted to a cash equivalent
static char const * const TREASURE_NAMES[NUM_TREASURES] = {
    "NULL TREASURE", "Flaming Torch", "Silver Key", "Gold Key", "Sword", "War Hammer", "Chain Mail Armor", "Shield",
    "Cloak of Protection", "Wand of Fireballs",
    "Emeralds", "Silver Rings", "Elven Amythests", "Diamond Dragon Eyes", "Crystal Ball", "Pieces of Eight",
    "Elemental Gems", "Shape-Shifting Stones", "Gold Dublooms"
};


static char const * const MONSTER_NAMES[NUM_MONSTERS] = {
    "NULL MONSTER",
    "Swashbuckler", "Werebear",
    "Caecliae", //   ???
    "Manticore", // the face of a human, the body of a lion, and the tail of a scorpion
    "Vampire",
    "Predebeast", // ???
    "Gargoyle",
    "Medusae",  // plural of Medusa, a jellyfish
    "Magi", // plural of Magus, priests in Zoroastrianism and earlier Iranian religions.
    "Fire Lizard",
    "Phase Spider", // ??? D&D A phase spider possesses the magical ability to phase in and out of the Ethereal Plane.
    "Troll", "Hell Hound", "Frost Giant", "Necromancer",
    "Hydra of 10 Heads", // ???
    "Patriach", // ???
    "Master Thief", "Living Statue"
};

enum Item {
    ITEM_NULL,
    ITEM_TORCH,
    ITEM_SLVER_KEY,
    ITEM_GOLD_KEY,
    ITEM_SWORD,
    ITEM_WAR_HAMMER,
    ITEM_CHAIN_MAIL,
    ITEM_SHIELD,
    ITEM_CLOAK,
    ITEM_WAND,
    ITEM_COUNT
};

constexpr int TREASURE_TORCH      = 1;
constexpr int TREASURE_SILVER_KEY = 2;
constexpr int TREASURE_GOLD_KEY   = 3;


struct GameState {
    const char * player_name;
    int room;  // current room
    int turns; // 1 point per turn
    int cash;

    int monsters_killed;  // number aliens/androids destroyed
    int monsters_fought;

    int magic;  // number of spells

    union {
        struct CharStats stats; // Named access: m.stats.strength
        struct { CHAR_STATS_UNION_BODY }; // Anonymous access: m.strength & m.as_array
    };

    bool has_torch;
    bool is_dead;
    bool completed; // true if reached final room

    // state for Mersenne Twister PRNG
    MTState mt_state;

    int  items[ITEM_COUNT];  // first 9 items of Treasure have a slot here with the same index
    bool rooms_visited[NUM_ROOMS];
};


//// ------------------------------------------------------------
////
////    PRNG - Mersenne Twister
////
//// ------------------------------------------------------------

// return random int in range [min_inclusive, max_exclusive)
static int rnd_range(struct GameState * gs, int min_inclusive, int max_exclusive) {
    return (int)mt_rand_range(&gs->mt_state, min_inclusive, max_exclusive);
}

// return random double in range [0,1)
static double rnd_d(struct GameState * gs) {
    return mt_random_double(&gs->mt_state);
}

//// ------------------------------------------------------------
////
////    DISPLAY FUNCTIONS
////
//// ------------------------------------------------------------


static void cls() {
    // \033[2J clears the screen, \033[H moves the cursor to the top-left corner
    printf("\033[2J\033[H");
    fflush(stdout);
}



// pass -1 to sleep default time of 30 miliseconds or last time set > 0
static void char_sleep(const int32_t microseconds) {
    constexpr int _30ms = 30'000;  // usleep() takes argument in microseconds
    static useconds_t current_delay = _30ms; // initial default

    useconds_t sleep_time;

    if (microseconds >= 0) {
        current_delay = microseconds;  // sticky sleep time
        sleep_time = microseconds;
    } else {
        sleep_time = current_delay;
    }

    if (sleep_time > 0) {
        usleep(sleep_time);
    }
}


//display string without adding newline
static void display(char const* msg) {
    fflush(stdout);
    for (char const *next = msg; *next; ++next) {
        putchar(*next);
        fflush(stdout);
        char_sleep(-1);
    }
}

//displays the string and adds newline to end.
static void display_line(char const* msg) {
    display(msg);
    putchar('\n');
    fflush(stdout);
    char_sleep(-1);
}

static void display_char_attributes(const struct CharStats stats) {
    display("Strength:  ");
    printf("%2d", stats.strength);
    display("  Charisma:     ");
    printf("%2d\n", stats.charisma);

    display("Dexterity: ");
    printf("%2d", stats.dexterity);
    display("  Intelligence: ");
    printf("%2d\n", stats.intelligence);

    display("Wisdom:    ");
    printf("%2d", stats.wisdom);
    display("  Constitution: ");
    printf("%2d\n", stats.constitution);
}


static void display_inventory(struct GameState * gs) {
    int item_count = 0;
    for (int bag_index = 0; bag_index < ITEM_COUNT; ++bag_index ) {
        if (gs->items[bag_index]) {
            display(TREASURE_NAMES[gs->items[bag_index]]);
            display(" - ");
            item_count++;
            if ( ! (item_count % 3) ) {
                display_line("");  // display 3 items per line
            }
        }
    }
    if (item_count) {
        if ( item_count % 3) {
            display_line("");
        }
    } else {
        display_line("You have no items.");
    }
}


static void display_status(struct GameState * gs) {
    display_line(gs->player_name);
    display("magic spells: ");
    printf("%d\n", gs->magic);

    if (!gs->cash) {
        display_line("You have no money.");
    } else {
        display("You have $");
        printf("%d.\n", gs->cash);
    }
}

static void display_random_room_text(struct GameState * gs, struct RandomTextArray *rta) {
    for (int i=0; i< rta->length; ++i) {
        const struct RandomText rt = rta->lines[i];
        const double random = mt_random_double(&gs->mt_state); // random double in [0,1)
        if (random < rt.chance_percent) {
            display_line(rt.text);
        } else if (rt.else_text) {
            display_line(rt.else_text);
        }
    }
}

static void display_room_desc(struct GameState * gs) {
    display_line("");
    if (!gs->has_torch && ROOM_GRAPH[gs->room][RGINDEX_TREASURE] != 1 ) {
        display_line("IT IS TOO DARK TO SEE ANYTHING!\n");
    } else {
        struct Room r = ROOMS[gs->room];
        if (r.preamble) {
            display_random_room_text(gs, r.preamble);
        }

        display_line(ROOMS[gs->room].desc);

        if (r.epilog) {
            display_random_room_text(gs, r.epilog);
        }
    }
}




static void display_room_monster(struct GameState * gs) {
    const int monster_index = ROOM_GRAPH[gs->room][RGINDEX_MONSTER];
    if ( monster_index == 0 ) {
        return;
    }
    display_line("");
    if (gs->has_torch ) {
        if (rnd_d(gs) < .5) {
            display("You come face to face with a ");
        } else {
            display("The room contains a ");
        }
        struct Room room =  ROOMS[gs->room];
        display(room.monster.name);
        display_line(":\n");
        display_char_attributes(room.monster.stats);
    } else {
        display_line("YOU FEEL A DANGEROUS PRESENCE!");
    }
}

static void display_room_treasure(struct GameState * gs) {
    const int treasure_index = ROOM_GRAPH[gs->room][RGINDEX_TREASURE];
    if ( treasure_index == 0 || (!gs->has_torch && gs->room != START_ROOM )) {
        return;
    }
    struct Room room =ROOMS[gs->room];
    display("You can see ");

    if (treasure_index > 9 ) {
        display(room.treasure.name);
        display(" worth $");
        printf("%d\n", room.treasure.value);
    } else {
        display_line(room.treasure.name);
    }
}

static void display_room_content(struct GameState * gs) {
    display_room_monster(gs);
    display_room_treasure(gs);
}


static void display_conclusion(const struct GameState * gs) {
    if (gs->completed && !gs->is_dead) {
        display("\nYou have succeeded, ");
        display_line(gs->player_name);
        display_line("You have escaped the Citadel of Pershu.");
        display_line("\nWell done!");
    } else if (gs->is_dead) {
        display_line("You have died.........");
    }
}

static int calc_score(const struct GameState * gs) ;
static int count_rooms_visited(const struct GameState * gs);

static void display_score(const struct GameState * gs) {
    display("\nSCORE: ");
    printf("%d\n", calc_score(gs));
    const int rooms_visited = count_rooms_visited(gs);
    printf("\nturns: %d, cash: %d, monsters fought: %d, killed: %d, rooms: %d\n",
        gs->turns, gs->cash, gs->monsters_fought, gs->monsters_killed, rooms_visited);
    printf("You completed %3.0f%% of the quest.\n", (double)rooms_visited * 100.0 / (NUM_ROOMS - NUM_DEATH_ROOMS) );
}

static void display_help_info(void) {
    display_line("\nVALID COMMANDS ARE:\n");

    display_line("[H]elp       [I]nventory  [Q]uit");
    display_line("[A]ttributes [T]ally");
    display_line("[R]etreat    [F]ight");
    display_line("[P]ick up    [G]et rid of");
    display_line("[N]orth      [S]outh");
    display_line("[E]ast       [W]est");
    display_line("[U]p         [D]own");
}





//// ------------------------------------------------------------
////
////    INPUT
////
//// ------------------------------------------------------------


static bool stdin_has_data(void) {
#ifdef _WIN32
    return _kbhit() != 0;
#else
    struct pollfd fds;
    fds.fd = STDIN_FILENO;
    fds.events = POLLIN;
    return poll(&fds, 1, 0) > 0;
#endif
}

static void flush_input(void) {
    while (stdin_has_data()) {
        int c = getchar();
        if (c == '\n' || c == EOF) break;
    }
}


struct StringBuffer {char buffer[1024];} get_str(char const *  prompt) {
    struct StringBuffer sb = {};
    display(prompt);

    if (fgets(sb.buffer, sizeof(sb.buffer), stdin)) {
        size_t len = strlen(sb.buffer);
        if (len > 0) {
            if (sb.buffer[len - 1] == '\n') {
                // Normal case: entire line read, remove newline
                sb.buffer[len - 1] = '\0';
            } else {
                // Truncation case: buffer was too small, leftovers remain in stdin
                flush_input();
            }
        }
    }
    return sb;
}

int get_int(char const * const prompt, const int min, const int max) {
    for (;;) {
        struct StringBuffer sb = get_str(prompt);

        // If the user just hit enter, sb.buffer[0] will be '\0'
        if (sb.buffer[0] == '\0') {
            display_line("INPUT CANNOT BE EMPTY. PLEASE ENTER A NUMBER.");
            continue;
        }

        char *endptr;
        long val = strtol(sb.buffer, &endptr, 10);

        // If endptr is the same as the buffer, no numbers were found at the start
        if (endptr == sb.buffer) {
            display_line("INVALID INPUT. PLEASE ENTER A VALID INTEGER.");
            continue;
        }

        if (val < min || val > max) {
            printf("OUT OF RANGE. PLEASE ENTER A NUMBER BETWEEN %d AND %d.\n", min, max);
            continue;
        }

        return (int)val;
    }
}


void display_command_err(char const * msg, char const command) {
    if (!msg) {
        msg = "INVALID COMMAND: ";
    }
    display(msg);
    printf("'%c'\n", command);
}

// return the first letter of the user's input converted to uppercase.
// input char must be in the `valid_chars` string to be accepted or user is re-prompted until it is
// err_msg may be null
char get_command_char(char const * const prompt, char const * const valid_chars, char const * const err_msg) {
    char first_letter;
    bool is_invalid_command;
    do {
        const struct StringBuffer sb = get_str(prompt);
        first_letter = (char)toupper(sb.buffer[0]);
        is_invalid_command = ! strchr(valid_chars, first_letter);
        if (is_invalid_command) {
            display_command_err(err_msg, first_letter);
        }
    } while (is_invalid_command);

    return first_letter;
}






//// ------------------------------------------------------------
////
////    INITIALIZE
////
//// ------------------------------------------------------------

struct RandomTextArray * create_rta(int length) {
    const size_t mem_size = sizeof(struct RandomTextArray) + sizeof(struct RandomText) * length;
    struct RandomTextArray * result = calloc(1, mem_size);
    result->length = length;
    return result;
}


static void init_rooms() {
    // randomized text in Rooms 1, 18, 37, 39
    // room 1
    ROOMS[1].epilog = create_rta(2);
    ROOMS[1].epilog->lines[0] = (struct RandomText){ .chance_percent = .5, .text="There is an exit to the west."};
    ROOMS[1].epilog->lines[1] = (struct RandomText){ .chance_percent = .5, .text="A tunnel leads to the south."};
    // room 18
    ROOMS[18].epilog = create_rta(1);
    ROOMS[18].epilog->lines[0] = (struct RandomText){ .chance_percent = .5, .text="A bat flies past you, shrieking."};
    // room 37
    ROOMS[37].epilog = create_rta(2);
    ROOMS[37].epilog->lines[0] = (struct RandomText){ .chance_percent = .7, .text="But now it tells you there is\na hidden stairwell in the room."};
    ROOMS[37].epilog->lines[1] = (struct RandomText){ .chance_percent = .3, .text="The voice faintly murmurs of the door to the south."};
    // room 39
    ROOMS[39].epilog = create_rta(1);
    ROOMS[39].epilog->lines[0] = (struct RandomText){ .chance_percent = .6, .text="A small door leaads to the north\nand another to the east."};



}

static int roll_d6(struct GameState * gs, int num_dice) {
    int result = 0;
    for (int i = 0; i < num_dice; ++i ) {
        result += rnd_range(gs, 1, 7);
    }

    return result;
}

static struct CharStats random_hero_stats(struct GameState * gs) {
    struct CharStats stats;
    stats.null_stat       = 0;
    stats.strength        = roll_d6(gs,3);
    stats.charisma        = roll_d6(gs,3);
    stats.dexterity       = roll_d6(gs,3);
    stats.intelligence    = roll_d6(gs,3);
    stats.wisdom          = roll_d6(gs,3);
    stats.constitution    = roll_d6(gs,3);
    return stats;
}

static struct CharStats random_monster_stats(struct GameState * gs) {
    struct CharStats stats;
    stats.null_stat       = 0;
    stats.strength        = 3  * rnd_range(gs, 0, 6) + 1;
    stats.charisma        = 3  * rnd_range(gs, 0, 6) + 1;
    stats.dexterity       = 3  * rnd_range(gs, 0, 6) + 1;
    stats.intelligence    = 3  * rnd_range(gs, 0, 6) + 1;
    stats.wisdom          = 3  * rnd_range(gs, 0, 6) + 1;
    stats.constitution    = 3  * rnd_range(gs, 0, 6) + 1;
    return stats;
}

static struct Treasure generate_treasure(struct GameState * gs, int treasure_index) {
    return (struct Treasure){
        .name = TREASURE_NAMES[treasure_index],
        .treasure_index = treasure_index,
        .value = rnd_range(gs, 0, 100 ) + 56};
}

static void initialize(struct GameState * gs) {
    mt_initialize_state(&gs->mt_state, 0);  // initialize the PRNG
    char_sleep(15'000); // set char delay to 15 ms

    init_rooms();

    gs->room = START_ROOM;
    gs->cash = 100;
    gs->magic = 3;

    // init stats
    gs->stats = random_hero_stats(gs);

    // allot monsters
    for (int monster_index= 1; monster_index <= 16; ++monster_index ) {
        for (;;) {
            int rand_room = rnd_range(gs, 1, 43 + 1);
            if ( ! ( ROOM_GRAPH[rand_room][RGINDEX_MONSTER] ||
                    rand_room == START_ROOM ||
                    rand_room == END_ROOM ||
                    rand_room == LIBRARY_ROOM ||
                    rand_room == GLOVE_STOREROOM)) {
                ROOM_GRAPH[rand_room][RGINDEX_MONSTER]  = monster_index;
                ROOMS[rand_room].monster =
                    (struct Monster){
                        .name = MONSTER_NAMES[monster_index],
                        .monster_index = monster_index,
                        .stats = random_monster_stats(gs)};
                break;;
            }
        }
    }
    // allot  treasure
    for (int treasure_index = 4; treasure_index < 19; ++treasure_index ) {
        for (;;) {
            int rand_room = rnd_range(gs, 1, 43 + 1);
            if ( ! ( ROOM_GRAPH[rand_room][RGINDEX_TREASURE] || rand_room == START_ROOM || rand_room == END_ROOM  ) ) {
                ROOM_GRAPH[rand_room][RGINDEX_TREASURE] = treasure_index;
                ROOMS[rand_room].treasure = generate_treasure(gs, treasure_index);
                break;
            }
        }
    }

    // special treasure items
    ROOMS[START_ROOM].treasure = generate_treasure(gs, TREASURE_TORCH);
    ROOMS[LIBRARY_ROOM].treasure = generate_treasure(gs, TREASURE_SILVER_KEY);
    ROOMS[GLOVE_STOREROOM].treasure = generate_treasure(gs, TREASURE_GOLD_KEY);




    cls();
    const struct StringBuffer sb = get_str("What is your name, explorer? ");
    char * new_str = malloc(strlen(sb.buffer) + 1);
    strcpy(new_str, sb.buffer);
    gs->player_name = new_str;

    display("Hello Explorer ");
    display_line(gs->player_name);
    display_line("Type '[H]elp' for a list of commands.");
}




//// ------------------------------------------------------------
////
////    CLEANUP
////
//// ------------------------------------------------------------

static void destroy_rooms() {
    for (int room_index = 0; room_index < NUM_ROOMS; ++room_index) {
        free(ROOMS[room_index].preamble);
        free(ROOMS[room_index].epilog);
    }
}

static void cleanup(struct GameState * gs) {
    destroy_rooms();
    free((void*)gs->player_name);
}


constexpr bool CONTINUE_GAME = true;
constexpr bool END_GAME = false;

char const * const VALID_COMMANDS = "HIQATRFPGNSEWUD";
char const * const VALID_DIRECTIONS = "NSEWUD";


//// ------------------------------------------------------------
////
////    GAME FUNCTIONS
////
//// ------------------------------------------------------------


static int count_rooms_visited(const struct GameState * gs) {
    int result = 0;
    for (int i = 0; i < NUM_ROOMS; ++i ) {
        result += gs->rooms_visited[i];
    }
    return result;
}


static int calc_score(const struct GameState * gs) {
    int sum_attributes = gs->stats.strength + gs->stats.charisma + gs->stats.dexterity +
        gs->stats.intelligence + gs->stats.wisdom + gs->stats.constitution;
    return 3 * gs->cash +  30 * gs->monsters_killed + 3 * sum_attributes + gs->turns  ;
}

static bool process_quit(const struct GameState * gs) {
    display_line("COWARD...QUITTER....TURNCOAT.....");
    // todo (rob) ask for confirmation?
    return END_GAME;
}


// checks if there is a monster and if so, that the user has selected either F or R. Returns true for success.
bool monster_check(struct GameState * gs, bool has_monster, char first_letter) {
    if ( has_monster && ( first_letter == 'F' || first_letter == 'R' )) {
        return true;
    }
    if (has_monster) {
        display_line("DANGER! You must either FIGHT or RETREAT.");
        return false;
    }
    return true;
}

// first_letter must be in "NSEWUD"
// return true if command was sucessfully processed. If false, the move is not allowed and an error message
// will have been displayed
static bool process_move_command(struct GameState * gs, char const first_letter) {
    const int location = gs->room;
    const int direction_index = calc_direction_index(first_letter);
    if (direction_index == DIRECTION_ERR) {
        display("Bad direction_index, first_letter='");
        printf("%c'\n", first_letter);
        return false;
    }

    if (ROOM_GRAPH[location][direction_index] > 0) {
        gs->room = ROOM_GRAPH[location][direction_index];
        return true;
    }

    display_line(BAD_MOVE_DESC[direction_index]);
    return false;
}

static void pick_up_treasure(struct GameState * gs) {
    const int treasure_index = ROOM_GRAPH[gs->room][RGINDEX_TREASURE];
    if (!treasure_index) {
        display_line("There is nothing to pick up.");
        return;
    }
    if ( !gs->has_torch && treasure_index != ITEM_TORCH ) {
        display_line("It is too dark to see anything.");
        return;
    }

    if ( treasure_index == ITEM_TORCH ) {
        gs->has_torch = true;
    }

    if (treasure_index > ITEM_WAND) {
        const struct Treasure treasure = ROOMS[gs->room].treasure;
        gs->cash += treasure.value;
    } else {
        gs->items[treasure_index] = treasure_index;
    }

    ROOM_GRAPH[gs->room][RGINDEX_TREASURE] = 0;
    ROOMS[gs->room].treasure = (struct Treasure){};
}

static void get_rid_of(struct GameState * gs) {
    display_line("I don't know how to get rid of anything yet, Dave.");
}



// clear the monster in the current room and its entry in the ROOMS array
static void clear_monster(struct GameState * gs) {
    ROOM_GRAPH[gs->room][RGINDEX_MONSTER] = 0;
    ROOMS[gs->room].monster = (struct Monster){};
}

static void process_fight(struct GameState * gs) {
    if (!ROOM_GRAPH[gs->room][RGINDEX_MONSTER]) {
        display_line("There is nothing to fight.");
        return;
    }
    struct Monster m = ROOMS[gs->room].monster;
    gs->monsters_fought++;

    display("\nYour opponent is a ");
    display_line(m.name);
    display_line("With the following attributes:");
    display_char_attributes(m.stats);
    display_line("\nYour attributes are:");
    display_char_attributes(gs->stats);

    int hero_tally = 0;
    int monster_tally = 0;

    if (gs->items[ITEM_SWORD]) {
        display_line("You have a sword");
        hero_tally++;
    }
    if (gs->items[ITEM_WAR_HAMMER]) {
        display_line("Your War Hammer will be of aid");
        hero_tally++;
    }
    if (gs->items[ITEM_CHAIN_MAIL]) {
        display_line("Chainmail armor gives you and edge");
        hero_tally++;
    }
    if (gs->items[ITEM_SHIELD]) {
        display("Your shield will help you in this fight against the ");
        display_line(m.name);
        hero_tally++;
    }
    if (gs->items[ITEM_CLOAK]) {
        display_line("The Cloak of Protection surrounds you");
        hero_tally++;
    }
    if (gs->items[ITEM_WAND]) {
        display_line("The Wand of Fireballs enhances your strength");
        hero_tally++;
    }
    if (gs->magic) {
        int choice = get_int("Enter 1 to fight with magic or 2 to rely on skill: ", 1, 2);
        if (choice == 1) {
            display_line("Your magic destroys it!");
            gs->magic--;
            gs->monsters_killed++;
            clear_monster(gs);
            return;
        }
    }

    display_line("Which attributes to fight with (choose 2):");
    display_line("1: STR, 2: CHA, 3: DEX, 4: INT, 5: WIS, 6: CON");

    const int first_skill = get_int("Enter first skill (1-6) ", 1, 6);
    int second_skill;
    for (;;) {
        second_skill = get_int("Enter second skill (1-6) ", 1, 6);
        if (first_skill != second_skill) {
            break;
        }
        display("Duplicate skill: ");
    }

    hero_tally += gs->stats.as_array[first_skill];
    hero_tally += gs->stats.as_array[second_skill];
    monster_tally += m.stats.as_array[first_skill];
    monster_tally += m.stats.as_array[second_skill];

    display("\nThe fight starts in favor of ");
    if (hero_tally > monster_tally ) {
        display_line("you.");
    } else {
        display_line(m.name);
    }
    display("The ");
    display(m.name);
    display(" - ");
    printf("%d\n",monster_tally);
    display(gs->player_name);
    display(" - ");
    printf("%d\n",hero_tally);


    for (;;) {
        int attack = rnd_range(gs, 0,8 );
        switch (attack) {
            case 0: {
                display_line("You get in a glancing blow");
                monster_tally--;
            } break;
            case 1: {
                display("The ");
                display(m.name);
                display_line(" strikes out!");
                hero_tally -=3;
                gs->stats.strength--;
                gs->stats.charisma--;
            } break;
            case 2: {
                display("You draw the ");
                display(m.name);
                display_line("'s blood!");
                monster_tally--;
            } break;
            case 3: {
                display_line("You are wounded!!");
                hero_tally -= rnd_range(gs, 1, 4);
                gs->dexterity --;  // annonymous union lets us do this!
            } break;
            case 4: {
                display("The ");
                display(m.name);
                display_line(" is tiring.");
                monster_tally--;
            } break;
            case 5: {
                display_line("You are bleeding....");
                hero_tally -= 2;
                gs->stats.wisdom--;
                gs->stats.constitution--;
            } break;
            case 6: {
                display("You wound the ");
                display(m.name);
                display_line("");
                monster_tally--;
            } break;
            case 7: {
                const int lost_cash = rnd_range(gs, 1, gs->cash + 1);
                display("It knocks $");
                printf("%d from your hand.\n",lost_cash);
                gs->cash -= lost_cash;
            } break;

            default: printf("Expected attack = [0,8), got %d,\n", attack);
        }

        if (! (hero_tally > 0 && monster_tally > 0 && rnd_d(gs) < .75 ) ) {
            break;
        }
    }

    if (hero_tally > monster_tally ) {
        display_line("You bested the beast!");
        gs->monsters_killed++;
    } else {
        display("The ");
        display(m.name);
        display_line(" got the better of you that time.");

        if (first_skill == STAT_STRENGTH || second_skill == STAT_STRENGTH ) {
            gs->stats.strength = 4 *  gs->stats.strength / 5;
        }
        if (first_skill == STAT_CHARISMA || second_skill == STAT_CHARISMA ) {
            gs->stats.charisma = 3 *  gs->stats.charisma / 4;
        }
        if (first_skill == STAT_DEXTERITY || second_skill == STAT_DEXTERITY ) {
            gs->stats.dexterity = 6 *  gs->stats.dexterity / 7;
        }
        if (first_skill == STAT_INTELLIGENCE || second_skill == STAT_INTELLIGENCE ) {
            gs->stats.intelligence = 2 *  gs->stats.intelligence / 3;
        }
        if (first_skill == STAT_WISDOM || second_skill == STAT_WISDOM ) {
            gs->stats.wisdom = 5 *  gs->stats.wisdom / 6;
        }
        if (first_skill == STAT_CONSTITUTION || second_skill == STAT_CONSTITUTION ) {
            gs->stats.constitution = 3 *  gs->stats.constitution / 6;
        }

    }

    clear_monster(gs);
    //normalize any negative stats to 0
    for (int i = 0; i < STAT_COUNT; ++i ) {
        if (gs->stats.as_array[i] < 0 ) {
            gs->stats.as_array[i] = 0;
        }
    }
}

static void process_retreat(struct GameState * gs) {
    const int room = gs->room;
    if (!ROOM_GRAPH[room][RGINDEX_MONSTER]) {
        display_line("There is nothing to retreat from.");
        return;
    }

    display_line("RUN AWAY!!!!!!!");
    // determine possible exits
    int num_exits = 0;
    int exits[RGINDEX_DOWN + 1] = {};
    for (int exit_index = RGINDEX_NORTH; exit_index <= RGINDEX_DOWN; ++exit_index ) {
        const int room_index = ROOM_GRAPH[room][exit_index];
        if ( room_index ) {
            //todo retreat through unlocked doors should be ok, but not through locked doors
            if ( !( room_index == END_ROOM || room_index ==  WINE_CELLAR_EAST || room_index == MARBLE_HALL) ) {
                // don't retreat to end room or through locked doors
                exits[num_exits++] = room_index;
            }
        }
    }

    // randomly move to an adjacent room. If current room has paths to itself, new room may not change
    int retreat_index = rnd_range(gs, 0, num_exits);

    if ( rnd_d(gs) < .6 || num_exits == 0 || retreat_index == room) {
        display_line("The creature blocks your path. You must fight.");
        process_fight(gs);
        return;
    }

    gs->room = exits[retreat_index];
}


bool main_game_loop(struct GameState * gs) {
    gs->turns++;
    gs->rooms_visited[gs->room] = true;

    printf("---------------------------------------------------------------------- %d\n", gs->turns);

    display_status(gs);
    display_line("");
    display_room_desc(gs);

    if (gs->room == END_ROOM || gs->room >= DROWNING_ROOM ) {
        if ( gs->room >= DROWNING_ROOM ) gs->is_dead = true;
        gs->completed = true;
        return END_GAME;
    }

    display_room_content(gs);

    const bool has_treasure = ROOM_GRAPH[gs->room][RGINDEX_TREASURE];
    const bool has_monster  = ROOM_GRAPH[gs->room][RGINDEX_MONSTER];

    char first_letter;
    bool is_invalid_command;

    do {
        is_invalid_command = false;

        flush_input();


        first_letter = get_command_char("\nWhat do you want to do? ", VALID_COMMANDS, nullptr);

        if (first_letter == 'Q') {
            return process_quit(gs);
        }

        if (!monster_check(gs, has_monster, first_letter) ) {
            is_invalid_command = true;
            continue;
        }

        if ( strchr(VALID_DIRECTIONS, first_letter) ) {
            //special logic for locked doors.
            if (gs->room == BEDCHAMBER_ROOM && first_letter == 'W' && !gs->items[ITEM_SLVER_KEY] ) {
                display_line("You need the Silver Key to unlock the door.");
                is_invalid_command = true;
                continue;
            }
            if (gs->room == SILVER_CROSSES_STOREROOM && first_letter == 'W' && !gs->items[ITEM_GOLD_KEY] ) {
                display_line("You need the Gold Key to unlock the door.");
                is_invalid_command = true;
                continue;
            }

            if ( process_move_command(gs, first_letter)) {
                return CONTINUE_GAME;
            } else {
                is_invalid_command = true;
                continue;
            }
        }
    } while (is_invalid_command);


    switch (first_letter) {
        case 'H':
            display_help_info();
            break;
        case 'I':
            display_inventory(gs);
            break;
        case 'A':
            display_char_attributes(gs->stats);
            break;
        case 'T' :
            display_score(gs);
            break;
        case 'P':
            pick_up_treasure(gs);
            break;
        case 'G':
            get_rid_of(gs);
            break;
        case 'R':
            process_retreat(gs);
            break;
        case 'F':
            process_fight(gs);
            break;

        default: display_command_err("UNHANDLED COMMAND: ", first_letter);

    }

    for (int i = STAT_STRENGTH; i < STAT_COUNT; ++i ) {
        if (gs->stats.as_array[i] == 0 ) {
            display_char_attributes(gs->stats);
            display_line("\nYour combined attributes are no longer\nenough to sustain you... You are dead.");
            gs->is_dead = true;
            gs->completed = true;
            return END_GAME;
        }
    }

    if (gs->room == GLOVE_STOREROOM ) {
        // if player is here, they already used the key to unlock the west door
        gs->items[ITEM_SLVER_KEY] = 0;
    }
    if (gs->room == MARBLE_HALL ) {
        gs->items[ITEM_GOLD_KEY] = 0;
    }


    return CONTINUE_GAME;
}

//// ------------------------------------------------------------
////
////    MAIN
////
//// ------------------------------------------------------------

int main(void) {
    setvbuf(stdin, nullptr, _IONBF, 0);
    struct GameState game_state = {};
    bool continue_loop;
    initialize(&game_state);
    do {
        continue_loop = main_game_loop(&game_state);
    } while (continue_loop);


    display_conclusion(&game_state);
    display_score(&game_state);
    cleanup(&game_state);
}
