//
// Created by Rob Ross on 1/19/26.
//
// to make: clang scrabble.c ../cs50.c  -o scrabble
//

#include <ctype.h>
#include <stdio.h>
#include "../cs50.h"


int score_word(string word) ;

// index 0 is for A, index 25 is for Z
const int LETTER_SCORES[26] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10};



int main(int argc, char *argv[]) {

    printf("Scrabble scoring app. Enter words for both players and word score is returned.\n");

    string word1 = get_string("Player 1: ");
    string word2 = get_string("Player 2: ");

    int player1_score = score_word(word1);
    int player2_score = score_word(word2);

    printf("Player 1 word: %s, score: %i\n", word1, player1_score);
    printf("Player 2 word: %s, score: %i\n", word2, player2_score);

    if (player1_score == player2_score) {
        printf("Tie!");
    }
    else if ( player1_score > player2_score) {
        printf("Player 1 wins!");
    }
    else {
        printf("Player 2 wins!");
    }
    printf("\n");

    return 0;
}

int score_word(string word) {
    int score = 0;
    int char_index = 0;
    while (word[char_index] != '\0') {
        if (isalpha(word[char_index])) {
            int char_lookup = toupper(word[char_index]) - 'A';
            score += LETTER_SCORES[char_lookup];
        }
        char_index++;
    }
    return score;
}

