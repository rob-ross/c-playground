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
// clang -fsanitize=address -fsanitize=leak -Wall -Werror -Wno-unused-const-variable -std=c23 -o citadel_of_pershu.out citadel_of_pershu.c mersenne_twister.c
//
//  PROD:
// clang -std=c23 -o citadel_of_pershu.out citadel_of_pershu.c mersenne_twister.c

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/_types/_useconds_t.h>

#include "mersenne_twister.h"


constexpr int NUM_ROOMS       = 48;
constexpr int START_ROOM      = 6;
constexpr int END_ROOM        = 31;
constexpr int LIBRARY_ROOM     = 4;
constexpr int GLOVE_STOREROOM = 21;
constexpr int DROWNING_ROOM    = 44;

constexpr int NUM_TREASURES  = 19;
constexpr int NUM_MONSTERS   = 20;

constexpr int BAG_SIZE = 9;


struct RandomText {
    const char *text;  // displayed if chance_percent is satisifed
    const char *else_text; // if not null, displayed when chance_percent not satisfied
    double chance_percent;  // betwen 0 and 1. Random number between 0 and 1  must be less (<) than this to be displayed
};

struct RandomTextArray {
    size_t length;
    struct RandomText lines[];  // flexible array
};

// Define the shared fields in a macro for reuse
#define CHAR_STATS_FIELDS \
int strength;         \
int charisma;         \
int dexterity;        \
int intelligence;     \
int wisdom;           \
int constitution;

struct CharStats {
    CHAR_STATS_FIELDS
};

struct Monster {
    char const * name;
    int monster_index;
    union {
        struct CharStats stats; // Named access: m.stats.strength
        struct {
            CHAR_STATS_FIELDS   // Anonymous access: m.strength
        };
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
{.id =  4,  .name= "ROOM 4",  .desc = "This is the Central Library. Leather-bound\nvolumes line the walls, right up to the\nornately carved ceiling..."},
{.id =  5,  .name= "ROOM 5",  .desc = "This room is an awful mess. It used to be\nan artist's studio. Paint and old\neasels lie around the floor."},
{.id =  6,  .name= "ROOM 6",  .desc = "This is the entrance to the Citadel of Pershu.\nTurn now, if you wish. Many stronger than you\nhave taken fright at its menacing towers and\ndark portals. If you wish to proceed, move\neast towards the black gaping doorway..."},
{.id =  7,  .name= "ROOM 7",  .desc = "A stone altar stands in the middle of the room\nwith two dead candles on it. An old book lies\non one part of the altar top, and a faded red\nparchment cloth covers the front of it."},
{.id =  8,  .name= "ROOM 8",  .desc = "You stand high on the black tower, the\nCitadel stretches to the north, south\nand east of you.\nThere is only one way out."},
{.id =  9,  .name= "ROOM 9",  .desc = "You are in the northern section of the\nCitadel's large wine cellar. Heavy\nbarrels lie all around you in this end\nof the cellar. There is a door to the north\nand one to the south."},
{.id = 10,  .name= "ROOM 10", .desc = "You are in the west wing of the wine\ncellar. There is a door to the west and\none to the east. The central circular\npart of the cellar lies beyond the\neast door."},
{.id = 11,  .name= "ROOM 11", .desc = "You are in the central circular\narea of the wine cellar. There is\na door at each compass point."},
{.id = 12,  .name= "ROOM 12", .desc = "You are in the east section of the\nwine cellar. There is a door to the\nwest and one - which you cannot use,\nas it only allows entrance to where\nyou now stand - to the east."},
{.id = 13,  .name= "ROOM 13", .desc = "There are many, many wine bottles here\nlying on their sides in this southern\nsection of the wine cellar. There is a\ndark, unfriendly-looking hole to the west\nand doors to the north and to the south."},
{.id = 14,  .name= "ROOM 14", .desc = "This is the Citadel's armory. Row upon row\nof shiny suits of armor are stored here..." },
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
{.id = 26,  .name= "ROOM 26", .desc = "You are in the storeroom for wooden\nboxes... There are two exxits." },
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
    {  0,  0, 27,  0,  0,  0,  0,  0 },  //  ROOM 26
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
    ITEM_DUMMY,
    ITEM_LIGHT,
    ITEM_ION,
    ITEM_LASER,
    ITEM_OXY,
    ITEM_TRANSPORTER,
    ITEM_SUIT,
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
        struct {
            CHAR_STATS_FIELDS   // Anonymous access: m.strength
        };
    };

    bool has_torch;
    bool is_dead;
    bool completed; // true if reached final room

    // state for Mersenne Twister PRNG
    MTState mt_state;

    int bag[BAG_SIZE];
    bool rooms_visited[NUM_ROOMS];
};


//// ------------------------------------------------------------
////
////    PRNG - Mersenne Twister
////
//// ------------------------------------------------------------

static int rnd_range(struct GameState * gs, int min_inclusive, int max_exclusive) {
    return mt_rand_range(&gs->mt_state, min_inclusive, max_exclusive);
}

static double rnd_d(struct GameState * gs) {
    return mt_random_double(&gs->mt_state);
}

//// ------------------------------------------------------------
////
////    DISPLAY FUNCTIONS
////
//// ------------------------------------------------------------


void cls() {
    // \033[2J clears the screen, \033[H moves the cursor to the top-left corner
    printf("\033[2J\033[H");
    fflush(stdout);
}



// pass -1 to sleep default time of 30 miliseconds or last time set > 0
void char_sleep(const int32_t microseconds) {
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
void display(char const* msg) {
    fflush(stdout);
    for (char const *next = msg; *next; ++next) {
        putchar(*next);
        fflush(stdout);
        char_sleep(-1);
    }
}

//displays the string and adds newline to end.
void display_line(char const* msg) {
    display(msg);
    putchar('\n');
    fflush(stdout);
    char_sleep(-1);
}

void display_char_stats(const struct CharStats stats) {
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

void display_status(struct GameState * gs) {
    display_line(gs->player_name);
    display("magic spells: ");
    printf("%d\n", gs->magic);
    display("monsters fought: ");
    printf("%d killed: %d\n", gs->monsters_fought, gs->monsters_killed);

    display_line("");
    display_char_stats(gs->stats);
    display_line("");

    int item_count = 0;
    for (int bag_index = 0; bag_index < BAG_SIZE; ++bag_index ) {
        if (gs->bag[bag_index]) {
            display(TREASURE_NAMES[gs->bag[bag_index]]);
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
        display_line("Your bag is empty.");
    }

    if (!gs->cash) {
        display_line("You have no money.");
    } else {
        display("You have $");
        printf("%d.\n", gs->cash);
    }
}

void display_random_room_text(struct GameState * gs, struct RandomTextArray *rta) {
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

void display_room_desc(struct GameState * gs) {
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




void display_room_monster(struct GameState * gs) {
    const int monster_index = ROOM_GRAPH[gs->room][RGINDEX_MONSTER];
    if ( monster_index == 0 ) {
        return;
    }

    if (gs->has_torch ) {
        if (rnd_d(gs) < .5) {
            display("You come face to face with a ");
        } else {
            display("The room contains a ");
        }
        struct Room room =  ROOMS[gs->room];
        display_line(room.monster.name);
        display_line("With attributes");
        display_char_stats(room.monster.stats);
    } else {
        display_line("YOU FEEL A DANGEROUS PRESENCE!");
    }
}

void display_room_treasure(struct GameState * gs) {
    const int treasure_index = ROOM_GRAPH[gs->room][RGINDEX_TREASURE];
    if ( treasure_index == 0 || (!gs->has_torch && gs->room != START_ROOM )) {
        return;
    }
    struct Room room =ROOMS[gs->room];
    display_line("You can see...");
    display_line(room.treasure.name);
    if (treasure_index > 9 ) {
        display("worth $");
        printf("%d\n", room.treasure.value);
    }
}

void display_room_content(struct GameState * gs) {
    display_room_monster(gs);
    display_room_treasure(gs);
}

void display_help_info(void) {
    display_line("\nVALID COMMANDS ARE:\n");

    display_line("[H]ELP     [I]NVENTORY  [Q]UIT");
    display_line("[B]UY      [O]XYGEN     [T]ALLY");
    display_line("[R]ETREAT  [F]IGHT");
    display_line("[P]ICK UP  [M]ATTER TRANSPORTER");
    display_line("[N]ORTH    [S]OUTH");
    display_line("[E]AST     [W]EST");
    display_line("[U]P       [D]OWN");
}





//// ------------------------------------------------------------
////
////    INPUT
////
//// ------------------------------------------------------------


void flush_input(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
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
}


static struct CharStats random_stats(struct GameState * gs) {
    struct CharStats stats;
    stats.strength        = 3  * rnd_range(gs, 0, 6) + 1;
    stats.charisma        = 3  * rnd_range(gs, 0, 6) + 1;
    stats.dexterity       = 3  * rnd_range(gs, 0, 6) + 1;
    stats.intelligence    = 3  * rnd_range(gs, 0, 6) + 1;
    stats.wisdom          = 3  * rnd_range(gs, 0, 6) + 1;
    stats.constitution    = 3  * rnd_range(gs, 0, 6) + 1;
    return stats;
}

static struct Treasure init_treasure(struct GameState * gs, int treasure_index) {
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
    gs->stats = random_stats(gs);

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
                        .stats = random_stats(gs)};
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
                ROOMS[rand_room].treasure = init_treasure(gs, treasure_index);
                break;
            }
        }
    }

    // special treasure items
    ROOMS[START_ROOM].treasure = init_treasure(gs, TREASURE_TORCH);
    ROOMS[LIBRARY_ROOM].treasure = init_treasure(gs, TREASURE_SILVER_KEY);
    ROOMS[GLOVE_STOREROOM].treasure = init_treasure(gs, TREASURE_GOLD_KEY);




    cls();
    const struct StringBuffer sb = get_str("WHAT IS YOUR NAME, EXPLORER? ");
    char * new_str = malloc(strlen(sb.buffer) + 1);
    strcpy(new_str, sb.buffer);
    gs->player_name = new_str;

    display("HELLO EXPLORER ");
    display_line(gs->player_name);
    display_line("TYPE 'HELP' FOR LIST OF COMMANDS.");



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


//// ------------------------------------------------------------
////
////    GAME FUNCTIONS
////
//// ------------------------------------------------------------

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

    return CONTINUE_GAME;
}

//// ------------------------------------------------------------
////
////    MAIN
////
//// ------------------------------------------------------------

int main(void) {
    struct GameState game_state = {};
    bool continue_loop;
    initialize(&game_state);
    do {
        continue_loop = main_game_loop(&game_state);
    } while (continue_loop);


    // display_conclusion(&game_state);
    //
    // display_tally(&game_state);
    cleanup(&game_state);
}
