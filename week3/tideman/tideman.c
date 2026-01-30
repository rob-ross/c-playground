/*
 * To d/l course files:
 *  curl -O https://cdn.cs50.net/2026/x/psets/3/tideman.zip
 *
 *  to build: clang tideman.c ../../cs50.c  -o tideman
 *  to run:   ./tideman name1[ name2 ... name9]
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
void display_rank_names(const int name_indices[], size_t n, const string names[]);
void display_int_array(const int int_array[], size_t n);
void display_2D_int_array(size_t n_rows, size_t n_cols, const int array[n_rows][n_cols]);
void display_pairs( size_t n, pair p[] );
void display_2D_bool_array( size_t n_rows, size_t n_cols, const bool array[n_rows][n_cols]);
void display_locked_array();

int vote(int rank, string name, int ranks[]);
int find(int i, const int int_array[], size_t size);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
int compare_pair(const void *a, const void *b);
void lock_pairs(void);
void print_winner(void);


int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: tideman [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    if (candidate_count < 2) {
        printf("You must specify at least 2 candidates, only %i names were provided\n", candidate_count);

        return 3;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i] = argv[i + 1];
    }

    // Clear graph of locked in pairs
    memset(locked, 0, sizeof locked);

    pair_count = 0;
    const int voter_count = get_int("Number of voters: ");

    // Query for votes
    for (int i = 0; i < voter_count; i++)
    {
        // ranks[i] is voter's ith preference
        int ranks[candidate_count];
        memset(ranks, -1, sizeof ranks);
        printf("\nVoter %i:\n", i + 1);  // use 1-indexed numbers for Voter message
        display_candidates(candidates, candidate_count);
        printf("----------------\n");
        // Query for each rank
        for (int j = 0; j < candidate_count; j++)
        {
            string name = get_string("Rank %i: ", j + 1);
            int vote_status = vote(j, name, ranks);
            while (vote_status != VALID)
            {
                if (vote_status == NO_SUCH_CANDIDATE) {
                    printf("\nInvalid vote, no candidate '%s'. Try again.\n", name);
                    display_candidates(candidates, candidate_count);
                } else {
                    printf("\nInvalid vote, already voted for candidate '%s'. Try again.\n", name);
                    display_rank_names(ranks, candidate_count, candidates);
                }

                name = get_string("Rank %i: ", j + 1);
                vote_status = vote(j, name, ranks);
            }
        }
        display_rank_names(ranks, candidate_count, candidates);
        printf("Displaying preferences:\n");
        display_2D_int_array(MAX, MAX, preferences);
        record_preferences(ranks);
        display_2D_int_array(MAX, MAX, preferences);


        printf("\n");
    }

    add_pairs();
    display_pairs(pair_count, pairs);

    sort_pairs();
    display_pairs(pair_count, pairs);
    display_locked_array();

    lock_pairs();
    display_locked_array();

    print_winner();
    return 0;
}

void display_candidates(const string names[], const size_t n) {
    printf("\nCandidates: ");
    for (int i = 0; i < n; i++) {
        printf("%s, ", names[i]);
    }
    printf("\n");
}

void display_rank_names(const int name_indices[], const size_t n, const string names[]) {
    printf("\nRank names: ");
    for (int i = 0; i < n; i++) {
        printf("%s, ", names[name_indices[i]]);
    }
    printf("\n");
}

void display_int_array(const int int_array[], const size_t n) {
    printf("\nElements of int array: \n");
    printf("-------------------------\n");
    for (int j = 0; j < n; j++) {
        printf("%i, ", int_array[j]);
    }
    printf("\n");
}

void display_2D_int_array(const size_t n_rows, const size_t n_cols, const int array[n_rows][n_cols]) {
    printf("\nElements of 2D int array: \n");
    printf("----------------------------\n");
    for (int row_index = 0; row_index < candidate_count; row_index++) {
        for ( int col_index = 0; col_index < candidate_count; col_index++) {
            printf("%3i ", array[row_index][col_index]);
        }
        printf("\n");
    }
}

void display_pairs(const size_t n, pair p[] ) {
    printf("\nElements of pair array: ");
    for (int i = 0; i < n; i++) {
        printf("( %i, %i ), ", p[i].winner, p[i].loser);
    }
    printf("\n");
}

void display_2D_bool_array(const size_t n_rows, const size_t n_cols, const bool array[n_rows][n_cols]) {
    printf("\nElements of 2D bool array: \n");
    printf("-----------------------------\n");
    for (int row_index = 0; row_index < candidate_count; row_index++) {
        for ( int col_index = 0; col_index < candidate_count; col_index++) {
            printf("%6s ", array[row_index][col_index] ? "true" : "false");
        }
        printf("\n");
    }
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

// Update ranks given a new vote
int vote(const int rank, const string name, int ranks[])
{
    for (int i = 0; i < candidate_count; i++)
    {
        // compare case-insensitively
        if ( strcasecmp(candidates[i], name) == 0  )
        {
            if (find(i, ranks, candidate_count) == NOT_FOUND) {
                ranks[rank] = i;
                return VALID;
            }
            return ALREADY_VOTED_FOR_CANDIDATE;
        }
    }
    return NO_SUCH_CANDIDATE;
}

int find(const int i, const int int_array[], const size_t size) {
    //find i in the array and return its index position, or return -1 if not found
    for (int j = 0; j < size; j++) {
        if (int_array[j] == i ) {
            return j;
        }
    }
    return NOT_FOUND;
}

// Update preferences given one voter's ranks
void record_preferences(int ranks[])
{
    // ranks is guaranteed to contain at least two valid candidate indices.
    // use a 2-element sliding window to get preference from each pair
    for ( int rank_index = 0; rank_index < candidate_count - 1; rank_index++ ) {
        for ( int j = rank_index + 1; j < candidate_count; j++) {
            preferences[ ranks[rank_index] ][ ranks[j] ]++;
        }
    }
}

// Record pairs of candidates where one is preferred over the other
void add_pairs(void) {
    for (int row = 0; row < candidate_count; row++) {
        for (int col = 0; col < candidate_count; col++) {
            if ( preferences[row][col] > 0 && preferences[row][col] > preferences[col][row]) {
                pairs[pair_count++] = (pair){ .winner = row, .loser = col};  // compound literal
            }
        }
    }
}

// Sort pairs in decreasing order by strength of victory
void sort_pairs(void)
{
    const size_t count = (size_t) candidate_count;
    qsort(pairs, count, sizeof(pair), compare_pair);
}

// Comparison function for qsort, descending on vote count
int compare_pair(const void *a, const void *b) {
    pair *p1 = (pair *)a;
    pair *p2 = (pair *)b;

    int a_votes = preferences[p1->winner][p1->loser];
    int b_votes = preferences[p2->winner][p2->loser];
    if ( a_votes == b_votes) {
        return 0;
    }
    if ( a_votes < b_votes ) {
        return 1;
    }
    return -1;
}

bool is_reachable(int target_cand_index, int start_cand_index) {
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
    // TODO
    return;
}
