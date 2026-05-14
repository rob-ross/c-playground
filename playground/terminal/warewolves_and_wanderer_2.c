// warewolves_and_wanderer.c
//
// Port of BASIC game from "Creating Adventure Games on Your Computer,"
// by Tim Hartnell, 1983
//
//
// Created 2026/05/01 18:00:08 PDT

// make :
// cd /Users/robross/Documents/Development/CLionProjects/CS50x/playground/terminal
//  DEBUG:
// clang -std=c23 -o warewolves_and_wanderer_2.out warewolves_and_wanderer_2.c mersenne_twister.c


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mersenne_twister.h"

enum Item {
    ITEM_DUMMY,
    ITEM_LIGHT,
    ITEM_AXE,
    ITEM_SWORD,
    ITEM_FOOD,
    ITEM_AMULET,
    ITEM_SUIT,
    ITEM_COUNT
};

struct GameState {
    int room; // current room
    const char * player_name;
    int tally; // 1 point per move
    int strength;
    int wealth;
    int food;
    int monsters_killed;

    // true when user has Item:
    bool items[ITEM_COUNT];

    bool is_dead;
    bool completed; // true if reached final room
};

struct Monster {
    int FF; // ferocity factor
    char const * name;
};

static struct Monster MONSTERS[4] = {
    { .FF =  5, .name = "FEROCIOUS WEREWOLF"},
    { .FF = 10, .name = "FANATICAL FLESHGORG"},
    { .FF = 15, .name = "MALOVENTY MALDEMER"},
    { .FF = 20, .name = "DEVASTATING ICE-DRAGON"},
};

enum Idx {
    NORTH, SOUTH, EAST, WEST, UP, DOWN, CONTENTS
};

// direction in "NSEWUD"
int calc_direction_index(char const direction) {
    switch (direction) {
        case 'N': return NORTH;
        case 'S': return SOUTH;
        case 'E': return EAST;
        case 'W': return WEST;
        case 'U': return UP;
        case 'D': return DOWN;
        default: return -1;
    }
}

static int ROOM_GRAPH[20][7] = {
    { 0,  0,  0,  0,  0,  0,  0}, // Room 0
    { 0,  2,  0,  0,  0,  0,  0}, // Room 1
    { 1,  3,  3,  0,  0,  0,  0}, // Room 2
    { 2,  0,  5,  2,  0,  0,  0}, // Room 3
    { 0,  5,  0,  0,  0,  0,  0}, // Room 4
    { 4,  0,  0,  3, 15, 13,  0}, // Room 5
    { 0,  0,  1,  0,  0,  0,  0}, // Room 6
    { 0,  8,  0,  0,  0,  0,  0}, // Room 7
    { 7, 10,  0,  0,  0,  0,  0}, // Room 8
    { 0, 19,  0,  8,  0,  8,  0}, // Room 9
    { 8,  0, 11,  0,  0,  0,  0}, // Room 10
    { 0,  0, 10,  0,  0,  0,  0}, // Room 11
    { 0,  0,  0, 13,  0,  0,  0}, // Room 12
    { 0,  0, 12,  0,  5,  0,  0}, // Room 13
    { 0, 15, 17,  0,  0,  0,  0}, // Room 14
    {14,  0,  0,  0,  0,  5,  0}, // Room 15
    {17,  0, 19,  0,  0,  0,  0}, // Room 16
    {18, 16,  0, 14,  0,  0,  0}, // Room 17
    { 0, 17,  0,  0,  0,  0,  0}, // Room 18
    { 9,  0, 16,  0,  0,  0,  0}, // Room 19
};

struct Room {
    int id;
    char const * name;
    char const * desc;
};

static struct Room ROOMS[20] = {
    {.id =  0,  .name= "ROOM 0", .desc = "UNUSED"},
    {.id =  1,  .name= "ROOM 1", .desc = "YOU ARE IN THE HALLWAY.\nTHERE IS A DOOR TO THE SOUTH.\nTHROUGH WINDOWS TO THE NORTH YOU CAN SEE A SECRET HERB GARDEN."},
    {.id =  2,  .name= "ROOM 2", .desc = "THIS IS THE AUDIENCE CHAMBER.\nTHERE IS A WINDOW TO THE WEST. BY LOOKING TO THE RIGHT,\nTHROUGH IT YOU CAN SEE THE ENTRANCE TO THE CASTLE.\nDOORS LEAVE THIS ROOM TO THE NORTH, EAST, AND SOUTH."},
    {.id =  3,  .name= "ROOM 3", .desc = "YOU ARE IN THE GREAT HALL, AN L-SHAPED ROOM.\nTHERE ARE DOORS TO THE EAST AND TO THE NORTH.\nIN THE ALCOVE IS A DOOR TO THE WEST."},
    {.id =  4,  .name= "ROOM 4", .desc = "THIS IS THE MONARCH'S PRIVATE MEETING ROOM.\nTHERE IS A SINGLE EXIT TO THE SOUTH."},
    {.id =  5,  .name= "ROOM 5", .desc = "THIS INNER HALLWAY CONTAINS A DOOR TO THE NORTH,\nAND ONE TO THE WEST, AND A CIRCULAR STAIRWELL\nPASSES THROUGH THE ROOM.\nYOU CAN SEE AN ORNAMENTAL LAKE THROUGH THE\nWINDOWS TO THE SOUTH."},
    {.id =  6,  .name= "ROOM 6", .desc = "YOU ARE AT THE ENTRANCE TO A FORBIDDING-LOOKING\nSTONE CASTLE. YOU ARE FACING EAST."},
    {.id =  7,  .name= "ROOM 7", .desc = "THIS IS THE CASTLE'S KITCHEN. THROUGH WINDOWS IN\nTHE NORTH WALL YOU CAN SEE A SECRET HERB GARDEN.\nA DOOR LEAVES THE KITCHEN TO THE SOUTH."},
    {.id =  8,  .name= "ROOM 8", .desc = "YOU ARE IN THE STORE ROOM, AMIDST SPICES,\nVEGETABLES, AND VAST SACKS OF FLOUR AND\nOTHER PROVISIONS. THERE IS A DOOR TO THE NORTH\nAND ONE TO THE SOUTH."},
    {.id =  9,  .name= "ROOM 9", .desc = "YOU HAVE ENTERED THE LIFT...\nIT SLOWLY DESCENDS..."},
    {.id = 10, .name= "ROOM 10", .desc = "YOU ARE IN THE REAR VESTIBULE.\nTHERE ARE WINDOWS TO THE SOUTH FROM WHICH\nYOU CAN SEE THE ORNAMENTAL LAKE.\nTHERE IS AN EXIT TO THE EAST, AND\nONE TO THE NORTH."},
    {.id = 11, .name= "ROOM 11", .desc = "EXIT"},
    {.id = 12, .name= "ROOM 12", .desc = "YOU ARE IN THE DANK, DARK DUNGEON.\nTHERE IS A SINGLE EXIT, A SMALL HOLE IN\nTHE WALL TOWARDS THE WEST."},
    {.id = 13, .name= "ROOM 13", .desc = "YOU ARE IN THE PRISON GUARDROOM, IN THE\nBASEMENT OF THE CASTLE. THE STAIRWELL\nENDS IN THIS ROOM. THERE IS ONE OTHER\nEXIT, A SMALL HOLE IN THE EAST WALL."},
    {.id = 14, .name= "ROOM 14", .desc = "YOU ARE IN THE MASTER BEDROOM ON THE UPPER\nLEVEL OF THE CASTLE....\nLOOKING DOWN FROM THE WINDOW TO THE WEST YOU\nCAN SEE THE ENTRANCE TO THE CASTLE, WHILE THE\nSECRET HERB GARDEN IS VISIBLE BELOW THE NORTH\nWINDOW. THERE ARE DOORS TO THE EAST AND\nTO THE SOUTH...."},
    {.id = 15, .name= "ROOM 15", .desc = "THIS IS THE L-SHAPPED UPPER HALLWAY.\nTO THE NORTH IS A DOOR, AND THERE IS A\nSTAIRWELL IN THE HALL AS WELL. YOU CAN SEE\nTHE LAKE THROUGH THE SOUTH WINDOWS."},
    {.id = 16, .name= "ROOM 16", .desc = "THIS ROOM WAS USED AS THE CASTLE TREASURY IN\nBY-GONE YEARS....\nTHERE ARE NO WINDOWS, JUST EXITS TO THE\nNORTH AND TO THE EAST."},
    {.id = 17, .name= "ROOM 17", .desc = "OOOOH... YOU ARE IN THE CHAMBERMAID'S BEDROOM.\nTHERE IS AN EXIT TO THE WEST AND A DOOR\nTO THE SOUTH...."},
    {.id = 18, .name= "ROOM 18", .desc = "THIS TINY ROOM ON THE UPPER LEVEL IS THE\nDRESSING CHAMBER. THERE IS A WINDOW TO THE\nNORTH, WITH A VIEW OF THE HERB GARDEN DOWN\nBELOW. A DOOR LEAVES TO THE SOUTH."},
    {.id = 19, .name= "ROOM 19", .desc = "THIS IS THE SMALL ROOM OUTSIDE THE CASTLE\nLIFT WHICH CAN BE ENTERED BY A DOOR TO THE NORTH.\nANOTHER DOOR LEADS TO THE WEST. YOU CAN SEE\nTHE LAKE THROUGH THE SOUTHERN WINDOWS."},
};


char const * const BAD_MOVE_DESC[6] = {
    "NO EXIT THAT WAY",
    "THERE IS NO EXIT SOUTH",
    "YOU CANNOT GO IN THAT DIRECTION",
    "YOU CANNOT MOVE THROUGH SOLID STONE",
    "THERE IS NO WAY UP FROM HERE",
    "YOU CANNOT DESCEND FROM HERE",
};

MTState mt_state;

// FORWARD REFERENCES
bool major_handling_routine(struct GameState * gs);
void display_room_desc(struct GameState * gs);
void initialize(struct GameState * gs);
void display_line(char const* msg);
void display(char const* msg);
void display_score(const struct GameState * gs);

int main(void) {
    mt_initialize_state(&mt_state, 0);
    // seed_rng_from_urandom();

    struct GameState game_state = { .room = 6, .strength = 105, .wealth = 75,  };
    bool continue_loop = true;
    initialize(&game_state);
    do {
        continue_loop = major_handling_routine(&game_state);
    } while (continue_loop);

    if (game_state.completed && !game_state.is_dead) {
        display_line("YOU'VE DONE IT!!");
        display_line("THAT WAS THE EXIT FROM THE CASTLE");
        display("\nYOU HAVE SUCCEEDED, ");
        display_line(game_state.player_name);
        display_line("\nYOU MANAGED TO GET OUT OF THE CASTLE");
        display_line("\nWELL DONE!");
    } else if (game_state.is_dead) {
        display_line("YOU HAVE DIED.........");
    }

    display_score(&game_state);
}



// Example usage to get a number between 1 and 19:
// int random_number = (mt_random_uint32(&mt_state) % 19) + 1;

void char_sleep(void) {
    constexpr int _30ms = 30'000;  // usleep() takes argument in microseconds
    usleep(_30ms);
}

//display string without adding newline
void display(char const* msg) {
    fflush(stdout);
    for (char const *next = msg; *next; ++next) {
        putchar(*next);
        fflush(stdout);
        char_sleep();
    }
}

//displays the string and adds newline to end.
void display_line(char const* msg) {
    display(msg);
    putchar('\n');
    fflush(stdout);
    char_sleep();
}

void display_command_err(char const * msg, char const command) {
    if (!msg) {
        msg = "INVALID COMMAND: ";
    }
    display(msg);
    printf("'%c'\n", command);
    char_sleep();
}

void display_score(const struct GameState * gs) {
    display_line("\nYOUR SCORE IS");
    int const score = 3* gs->tally + 5* gs->strength + 2* gs->wealth + gs->food + 30*gs->monsters_killed;
    printf("%d", score);
    char_sleep();
}

void display_inventory(struct GameState * gs) {
    if (gs->wealth > 0) {
        display("YOU HAVE $");
        printf("%d", gs->wealth);
        char_sleep();
        display_line(" WEALTH.");
    }

    if (gs->food > 0 ) {
        display("YOUR PROVISIONS SACK HOLDS ");
        printf("%d",gs->food);
        char_sleep();
        display_line(" UNITS OF FOOD");
    }

    if (gs->items[ITEM_SUIT]) {
        display_line("YOU ARE WEARING ARMOR.");
    }

    int num_items = gs->items[ITEM_AXE] + gs->items[ITEM_SWORD]  + gs->items[ITEM_AMULET];

    if (num_items > 0) {
        display("YOU ARE CARRYING ");
    }

    // grammar : commas and conjunctions
    // NOTE (rob) - This won't scale well when adding items.
    if (num_items == 1) {
        if (gs->items[ITEM_AXE])         display_line("AN AXE.");
        else if (gs->items[ITEM_SWORD])  display_line("A SWORD.");
        else if (gs->items[ITEM_AMULET]) display_line("THE MAGIC AMULET.");
    }

    if (num_items == 3) {
        display_line("AN AXE, A SWORD, AND THE MAGIC AMULET.");
    }

    if (num_items == 2) {
        if (gs->items[ITEM_AXE]) {
            display("AN AXE AND");
            if (gs->items[ITEM_SWORD]) display_line(" A SWORD.");
            else display_line(" THE MAGIC AMULET.");
        } else if (gs->items[ITEM_SWORD]) {
            display_line("A SWORD AND THE MAGIC AMULET.");
        }
    }

    display_line("");
}


//990 REM ROOM DESCRIPTION
void display_room_desc(struct GameState * gs) {
    display_line("");
    display_line(ROOMS[gs->room].desc);
    if (gs->room == 9) {
        // if in Room 9 transition to Room 10 and show description
        gs->room = 10;
        display_room_desc(gs);
    }
}

// first_letter must be in "NSEWUD"
// return true if command was sucessfully processed. If false, the move is not allowed.
bool process_move_command(struct GameState * gs, char first_letter) {

    int location = gs->room;
    int direction_index = calc_direction_index(first_letter);
    if (ROOM_GRAPH[location][direction_index] > 0) {
        gs->room = ROOM_GRAPH[location][direction_index];
        if (gs->room == 11) {
            // Exit!!

            gs->completed = true;
        }
        return true;
    }

    display_line(BAD_MOVE_DESC[direction_index]);


    return false;
}

int const ITEM_COSTS[] = { 0, 15, 10, 20, 2, 30, 50};

void display_inventory_menu(struct GameState * gs) {
    display("\nYOU HAVE $");
    printf("%d\n",gs->wealth);
    char_sleep();


    display_line("YOU CAN BUY 1 - FLAMING TORCH ($15)");
    display_line("            2 - AXE ($10)");
    display_line("            3 - SWORD ($20)");
    display_line("            4 - FOOD ($2 PER UNIT)");
    display_line("            5 - MAGIC AMULET ($30)");
    display_line("            6 - SUIT OF ARMOR ($50)");
    display_line("            0 - TO CONTINUE ADVENTURE");
}

void do_inventory(struct GameState * gs) {
    display_line("PROVISIONS AND INVENTORY");
    if (gs->wealth <=0 ) {
        display_line("YOU HAVE NO MONEY.");
        return;
    }

    for (;;) {
        display_inventory_menu(gs);

        char option;
        fflush(stdin);
        do {
            display("ENTER NO. OF ITEM REQUIRED ");
            option = (char)getchar();
            fflush(stdin);
        } while ( !(option >= '0' && option <= '6') );

        const int option_index = option - '0';

        // printf("You selected ** %c ** \n", option);

        if ( option_index == 0 ) {
            //cls
            break;
        }

        if ( option_index != 4 ) {
            gs->wealth -= ITEM_COSTS[option_index];
            gs->items[option_index] = true;
            if (gs->wealth < 0) {
                display_line("YOU HAVE TRIED TO CHEAT ME!");
                //punish user
                gs->wealth = 0;
                for (int i = 0; i < ITEM_COUNT; ++i) {
                    gs->items[i] = false;  // no soup for you!
                }
                gs->food = gs->food / 4 ;
            }
        }

        if (option_index == 4 ) {
            for (;;) {
                char food_quantity;
                fflush(stdin);
                do {
                    display("HOW MANY UNITS OF FOOD (0-9)? ");
                    food_quantity = (char)getchar();
                    fflush(stdin);
                } while ( !(food_quantity >= '0' && food_quantity <= '9') );

                const int qty = food_quantity - '0';
                int cost = qty * ITEM_COSTS[ITEM_FOOD];
                if (gs->wealth - cost < 0 ) {
                    display_line("YOU HAVEN'T GOT ENOUGH MONEY!");
                } else {
                    gs->wealth -= cost;
                    gs->food += qty;
                    break;
                }
            }
        }
    }

}


void eat_food(struct GameState * gs) {
    if (gs->food <= 0) return;
    for (;;) {
        char food_quantity;
        fflush(stdin);
        do {
            display("YOU HAVE ");
            printf("%d", gs->food);
            char_sleep();
            display_line(" UNITS OF FOOD.");
            display("HOW MANY DO YOU WANT TO EAT (0-9)? ");

            food_quantity = (char)getchar();
            fflush(stdin);
        } while ( !(food_quantity >= '0' && food_quantity <= '9') );

        const int qty = food_quantity - '0';
        if ( qty <= gs->food) {
            gs->food -= qty;
            gs->strength += (5 * qty);
            break;
        }
    }
}

void pick_up_treasure(struct GameState * gs) {
    if ( ROOM_GRAPH[gs->room][CONTENTS] <= 0 ) {
        display_line("THERE IS NO TREASURE TO PICK UP.");
        return;
    }
    if ( !gs->items[ITEM_LIGHT] ) {
        display_line("YOU CANNOT SEE WHERE IT IS");
        return;
    }
    gs->wealth += ROOM_GRAPH[gs->room][CONTENTS];
    ROOM_GRAPH[gs->room][CONTENTS] = 0;
}

void use_magic_amulet(struct GameState * gs) {
    for (;;) {
        // Generate a random number between 1 and 19
        int room_index = (int)(mt_random_uint32(&mt_state) % 19) + 1;
        if ( !(room_index == 6 || room_index == 11 )) {
            gs->room = room_index;
            break;
        }
    }
}

void fight(struct GameState * gs) {
    if (ROOM_GRAPH[gs->room][CONTENTS] >= 0) {
        return; // no monster to fight
    }
    int const monster_index = -ROOM_GRAPH[gs->room][CONTENTS];
    struct Monster const monster = MONSTERS[ monster_index ];
    int ferocity_factor = monster.FF;

    if (gs->items[ITEM_SUIT]) {
        display_line("YOUR ARMOR INCREASES YOUR CHANCE OF SUCCESS.");
        ferocity_factor = 3 * (ferocity_factor / 4);  //armor gives 25% more advantage
    }

    for (int j = 0; j < 6; ++j ) {
        display_line("*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*");
    }

    const bool has_axe   = gs->items[ITEM_AXE];
    const bool has_sword = gs->items[ITEM_SWORD];

    if ( !has_axe && !has_sword) {
        display_line("YOU HAVE NO WEAPONS.\nYOU MUST FIGHT WITH BARE HANDS.");
        ferocity_factor = ferocity_factor + ferocity_factor / 5;
    } else if ( has_axe && !has_sword) {
        display_line("YOU HAVE ONLY AN AXE TO FIGHT WITH.");
        ferocity_factor = 4 * ferocity_factor / 5;
    } else if ( !has_axe && has_sword) {
        display_line("YOU MUST FIGHT WITH YOUR SWORD.");
        ferocity_factor = 3 * ferocity_factor / 4;
    } else {
        char option;
        fflush(stdin);
        do {
            display("WHICH WEAPON? 1 - AXE, 2 - SWORD ");
            option = (char)getchar();
            fflush(stdin);
        } while (option != '1' && option != '2');

        if (option == '1') {
            ferocity_factor = 4 * ferocity_factor / 5;
        } else {
            ferocity_factor = 3 * ferocity_factor / 4;
        }

    }

    do {
        if ( mt_random_uint32(&mt_state) % 2 == 1) {
            display_line("");
            display(monster.name);
            display_line(" ATTACKS.");
        } else {
            display_line("\nYOU ATTACK.");
        }

        if ( mt_random_uint32(&mt_state) % 2 == 1 ) {
            display_line("YOU MANAGE TO WOUND IT.");
            ferocity_factor = 5 * ferocity_factor / 6;
        }

        if ( mt_random_uint32(&mt_state) % 2 == 1 ) {
            display_line("THE MONSTER WOUNDS YOU!");
            gs->strength -= 5;
        }
    } while ( mt_random_uint32(&mt_state) % 100 > 34);

    if ( mt_random_uint32(&mt_state) % 16 > ferocity_factor) {
        display("\n\nAND YOU MANAGED TO KILL THE ");
        display_line(monster.name);
        gs->monsters_killed++;
    } else {
        display("\n\nTHE ");
        display(monster.name);
        display_line(" DEFEATED YOU!");
        gs->strength /= 2;
    }

    ROOM_GRAPH[gs->room][CONTENTS] = 0;
}

char const * const VALID_COMMANDS = "QNSEWUDRFICPM";
char const * const VALID_DIRECTIONS = "NSEWUD";

void retreat(struct GameState * gs) {
    if (ROOM_GRAPH[gs->room][CONTENTS] >= 0) {
        return; // no monster to retreat from
    }
    if ( (mt_random_uint32(&mt_state) % 100) > 69 ) {
        display_line("NO, YOU MUST STAND AND FIGHT!");
        fight(gs);
        return;
    }



    char first_letter;
    bool is_invalid_command = false;
    char input_buffer[1024];
    // process the next user command. First, check the command is valid for the current state
    do {
        display("WHICH WAY DO YOU WANT TO FLEE? ");
        fscanf(stdin, "%s", input_buffer);
        // printf("   you entered: %s\n", input_buffer);
        first_letter = (char)toupper(input_buffer[0]);

        is_invalid_command = ! strchr(VALID_DIRECTIONS, first_letter);
        if (is_invalid_command) {
            display_command_err("INVALID DIRECTION: ", first_letter);
        }
    } while (is_invalid_command || !process_move_command(gs, first_letter) );


}

// 160 REM MAJOR HANDLING ROUTINE
// returns true if still alive
bool major_handling_routine(struct GameState * gs) {
    display_line("\n\n----------------- -------------------");
    gs->strength -= 5;

    if (gs->strength <= 15) {
        display("WARNING, ");
        display(gs->player_name);
        display_line("YOUR STRENGTH\nIS RUNNING LOW");
    }
    if (gs->strength <= 0) {
        //goto 2300
        //REM DEATH
        gs->is_dead = true;
        return false; // is dead
    }

    gs->tally += 1;
    display(gs->player_name);
    display(", YOUR STRENGTH IS ");
    printf("%d.\n", gs->strength);
    char_sleep();

    display_inventory(gs);

    if (gs->items[ITEM_LIGHT]) {
        //GOSUB 990:REM ROOM DESCRIPTION
        display_room_desc(gs);
        putchar('\n');
    } else {
        display_line("IT IS TOO DARK TO SEE ANYTHING\n");
    }


    int room_contents = ROOM_GRAPH[gs->room][CONTENTS];
    if ( room_contents > 0 ) {
        display("THERE IS TREASURE HERE WORTH $");
        printf("%d\n", room_contents);
        char_sleep();
    } else if (room_contents < 0 ) {
        int index = -room_contents;
        struct Monster monster = MONSTERS[index];
        display_line("\n\nDANGER...THERE IS A MONSTER HERE....");
        display("\nIT IS A ");
        display_line(MONSTERS[index].name);
        display("\nTHE DANGER LEVEL IS ");
        printf("%d!!\n", monster.FF);
        char_sleep();
    }




    char input_buffer[1024];  // there's an env variable for terminal that defines this
    // fscanf(stdin, "%s", input_buffer);
    // printf("   you entered: %s\n", input_buffer);
    putchar('\n');

    char first_letter;
    bool is_invalid_command = false;

    // process the next user command. First, check the command is valid for the current state
    do {
        display("WHAT DO YOU WANT TO DO? ");
        fscanf(stdin, "%s", input_buffer);
        // printf("   you entered: %s\n", input_buffer);
        first_letter = (char)toupper(input_buffer[0]);

        is_invalid_command = ! strchr(VALID_COMMANDS, first_letter);

        if (is_invalid_command) {
            display_command_err(nullptr, first_letter);
            continue;
        }

        if (first_letter == 'Q') {
            return false; // quit game
        }


        if (room_contents < 0 &&
            !( first_letter == 'F' || first_letter == 'R' ) ) {
            // if monster, can only Fight or Retreat
            display_line("MONSTER! YOU MUST EITHER FIGHT OR RETREAT.");
            is_invalid_command = true;
            continue;
        }

        if (room_contents >= 0 &&
            ( first_letter == 'F' || first_letter == 'R' )) {
            // nothing to fight
            display_line("THERE IS NO MONSTER.");
            is_invalid_command = true;
            continue;
        }

        if (first_letter == 'C' && gs->food == 0) {
            display_line("YOU HAVE NO FOOD.");
            is_invalid_command = true;
            continue;
        }

        if ( strchr(VALID_DIRECTIONS, first_letter) ) {
            int direction_index = calc_direction_index(first_letter);
            if (ROOM_GRAPH[gs->room][direction_index] == 0) {
                printf("%s\n", BAD_MOVE_DESC[direction_index]);
                is_invalid_command = true;
                continue;
            }
            is_invalid_command = false;
            break;

        }

    } while (is_invalid_command);

    // Now process the command

    // display_line("\n\n----------------- -------------------");

    if (strchr(VALID_DIRECTIONS, first_letter) ) {
        // move command
        int direction_index = calc_direction_index(first_letter);
        gs->room = ROOM_GRAPH[gs->room][direction_index];
        if (gs->room == 11) {
            // Exit!!
            gs->completed = true;
            return false;
        }
        return true;
    }




    switch (first_letter) {
        case 'I':
            //INVENTORY/PROVISIONS
            do_inventory(gs);
            break;
        case 'C' :
            eat_food(gs);
            break;
        case 'P':
            pick_up_treasure(gs);
            break;
        case 'M':
            use_magic_amulet(gs);
            break;
        case 'R':
            retreat(gs);
            break;
        case 'F':
            fight(gs);
            break;

        default: display_command_err("UNEXPECTED COMMAND: ", first_letter);

    }


    return true; // continue game
}


void initialize(struct GameState * gs) {
    char name_buffer[1024];
    display("WHAT IS YOUR NAME, EXPLORER? ");
    scanf("%s", name_buffer);
    fflush(stdin);
    char * new_str = malloc(strlen(name_buffer) + 1);
    strcpy(new_str, name_buffer);
    gs->player_name = new_str;
    //allot treasure
    for (int j = 0; j < 4; ++j ) {
        for (;;) {
            // Generate a random number between 1 and 19
            uint32_t room_index = (mt_random_uint32(&mt_state) % 19) + 1;
            if ( !(room_index == 6 || room_index == 11 || ROOM_GRAPH[room_index][CONTENTS] !=0 ) ) {
                const int treasure = (int)(mt_random_uint32(&mt_state) % 100) + 10; // rand val between 10 and 109 inclusive
                ROOM_GRAPH[room_index][CONTENTS] = treasure;
                break;
            }

        }
    }
    //allot monsters
    for (int j = 0; j < 4; ++j ) {
        for (;;) {
            // Generate a random number between 1 and 19
            int room_index = (int)(mt_random_uint32(&mt_state) % 19) + 1;
            if ( !(room_index == 6 || room_index == 11 || ROOM_GRAPH[room_index][CONTENTS] !=0 ) ) {
                ROOM_GRAPH[room_index][CONTENTS] = -j;
                break;
            }
        }
    }
    // rooms 4 and 16 get special treasures
    ROOM_GRAPH[4][CONTENTS] = 100 + (int)(mt_random_uint32(&mt_state) % 100);
    ROOM_GRAPH[16][CONTENTS] = 100 + (int)(mt_random_uint32(&mt_state) % 100);
}

