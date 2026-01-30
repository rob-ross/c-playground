//
// Created by Rob Ross on 1/24/26.
/*
*  to build: clang test_is_reachable.c ../../cs50.c  -o test_is_reachable
*  to run:   ./test_is_reachable
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../cs50.h"

// Max number of candidates
#define MAX 9

// value returned from find when element not found
#define NOT_FOUND -1

// voting constants
#define VALID 0
#define NO_SUCH_CANDIDATE -1
#define ALREADY_VOTED_FOR_CANDIDATE -2

// preferences[i][j] is number of voters who prefer i over j
int preferences[MAX][MAX];

// locked[i][j] means i is locked in over j
bool locked[MAX][MAX];

// Each pair has a winner, loser
typedef struct
{
    int winner;
    int loser;
} pair;

// Array of candidates
string candidates[MAX];
pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

// Function prototypes
void display_candidates(const string names[], size_t n);
void display_pairs( size_t n, pair p[] );
void display_locked_array();
void lock_pairs(void);

bool is_reachable(int target_cand_index, int start_cand_index) ;
void print_winner(void);


int main(int argc, char* argv[]) {
    candidates[candidate_count++] = "Alice";
    candidates[candidate_count++] = "Bob";
    candidates[candidate_count++] = "Charlie";



    // test pair data: ( 0, 1 ), ( 2, 0 ), ( 1, 2 ),
    pairs[pair_count++] = (pair){ .winner = 0, .loser = 1};
    pairs[pair_count++] = (pair){ .winner = 2, .loser = 0};
    pairs[pair_count++] = (pair){ .winner = 1, .loser = 2};

    memset(locked, 0, sizeof locked);

    locked[0][1] = true;
    locked[2][0] = true;
    display_pairs(pair_count, pairs);
    display_locked_array();

    const int target = 1;
    const int start = 2;
    const bool flag = is_reachable(target, start);
    printf("is '%s' reachable from '%s'?  %s\n", candidates[target], candidates[start],flag ? "true" : "false");

    lock_pairs();
    display_locked_array();

    print_winner();
}

void display_candidates(const string names[], const size_t n) {
    printf("\nCandidates: ");
    for (int i = 0; i < n; i++) {
        printf("%s, ", names[i]);
    }
    printf("\n");
}

void display_pairs(const size_t n, pair p[] ) {
    printf("\nElements of pair array: ");
    for (int i = 0; i < n; i++) {
        printf("( %i, %i ), ", p[i].winner, p[i].loser);
    }
    printf("\n");
}

void display_locked_array() {
    printf("\nElements of locked array: \n       ");
    for (int i = 0; i < candidate_count; i++) {
        printf("%-6.6s ", candidates[i]);
    }
    printf("\n-----------------------------\n");
    for (int row_index = 0; row_index < candidate_count; row_index++) {
        printf("%-6.6s ", candidates[row_index]);
        for ( int col_index = 0; col_index < candidate_count; col_index++) {
            printf("%-6.6s ", locked[row_index][col_index] ? "true" : "false");
        }
        printf("\n");
    }
}

bool is_reachable(const int target_cand_index, const int start_cand_index) {
    // traverse the locked array, visiting any node marked true, and if we can reach the argument node,
    // return true
    for (int visit_node = 0; visit_node < candidate_count; visit_node++) {
        if (locked[start_cand_index][visit_node]) {
            if (locked[start_cand_index][visit_node] == target_cand_index) {
                // base case
                return true;
            }
            return is_reachable(target_cand_index, visit_node);
        }
    }

    return false;
}

// Lock pairs into the candidate graph in order, without creating cycles
void lock_pairs(void)
{
    for (int i = 0; i < pair_count; i++) {
        // if this pair doesn't cause a cycle, add it
        if (! is_reachable( pairs[i].winner, pairs[i].loser )) {
            locked[ pairs[i].winner ][ pairs[i].loser ] = true; // edge from winner to loser
        }
    }
}

// Print the winner of the election
void print_winner(void)
{
    int source = -1;
    /// the winner is the canidate with no edges to it in the locked array.
    for ( int col = 0; col < candidate_count; col++) {
        for (int row = 0; row < candidate_count; row++) {
            if ( locked[row][col] ) {
                // there is an edge to this node, so it can't be source
                goto continue_outer;
            }
        }
        // if we got here, no nodes in the current colum were true, thus there is no edge to this node.
        // this is the source.
        source = col;
        break;
        continue_outer:
                ; //nop
    }

    printf("\nWinner of Tideman ranked-voting election: %s\n", candidates[source]);
}