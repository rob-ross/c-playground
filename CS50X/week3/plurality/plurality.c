/*
 * to build: clang plurality.c ../../cs50.c -o plurality
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include "../../cs50.h"

// Max number of candidates
#define MAX 9

// Candidates have name and vote count
typedef struct
{
    string name;
    int votes;
} candidate;

// Array of candidates
candidate candidates[MAX];

// Number of candidates
int candidate_count;

typedef int (*compare_func)(const void *, const void *);


// Function prototypes
void sort_candidates_by_votes(void);
void display_candidates(void);
bool vote(string name);
void print_winner(void);


int main(int argc, string argv[])
{
    // Check for invalid usage
    if (argc < 2)
    {
        printf("Usage: plurality [candidate ...]\n");
        return 1;
    }

    // Populate array of candidates
    candidate_count = argc - 1;
    if (candidate_count > MAX)
    {
        printf("Maximum number of candidates is %i\n", MAX);
        return 2;
    }
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i].name = argv[i + 1];
        candidates[i].votes = 0;
    }

    display_candidates();

    int voter_count = get_int("Number of voters: ");
    printf("\n");
    // Loop over all voters
    for (int i = 0; i < voter_count; i++)
    {
        string name = get_string("Vote: ");

        // Check for invalid vote
        if (!vote(name))
        {
            printf("\nInvalid vote '%s'.\n\n", name);
        }
    }

    // Display winner of election
    print_winner();
}

void display_candidates(void) {
    printf("\nCandidates:\n");
    printf("-----------\n");
    for (int i = 0; i < candidate_count; i++) {
        printf("Name: %s, Votes %i\n", candidates[i].name, candidates[i].votes);
    }
    printf("\n");
}

// Comparison function for lfind
int compare_candidate_names(const void *a, const void *b)
{
    // a is the 'key' (the string we are looking for)
    // b is a pointer to an element in the 'candidates' array
    const string search_name = (string) a;
    const candidate *c = (candidate *) b;

    return strcmp(search_name, c->name);
}

// Comparison function for qsort
int compare_candidate_votes(const void *a, const void *b) {
    const candidate *candidate_a = (candidate *) a;
    const candidate *candidate_b = (candidate *) b;

    if (candidate_a->votes == candidate_b->votes) {
        return 0;
    }
    if (candidate_a->votes < candidate_b->votes) {
        return 1;
    }
    return -1;
}

void sort_candidates_by_votes(void) {
    size_t count = (size_t) candidate_count;
    qsort(candidates, count, sizeof(candidate), compare_candidate_votes  );
}

// Update vote totals given a new vote
bool vote(string name)
{
    // We need a pointer to the count because lfind expects size_t *
    // Since candidate_count is an int, we should be careful with types,
    // but for this exercise, we can cast the address.
    size_t count = (size_t) candidate_count;
    //find the candidate element in candidates by name
    candidate *c =  lfind(name, candidates, &count, sizeof(candidate), compare_candidate_names);
    if (c == NULL) {
        return false;
    }

    c->votes++;

    return true;
}

// Print the winner (or winners) of the election
void print_winner(void)
{
    int most_votes = 0;

    if (candidate_count == 0) {
        printf("No candidates exist.\n");
        return;
    }

    sort_candidates_by_votes();
    display_candidates();

    most_votes = candidates[0].votes;
    if (most_votes == 0) {
        printf("No valid votes were cast.\n");
        return;
    }
    printf("Winner(s):\n");
    printf("------------\n");
    int index = 0;
    while (index < candidate_count && candidates[index].votes == most_votes ) {
        printf("%s, %i votes\n", candidates[index].name, candidates[index].votes);
        index++;
    }

    return;
}