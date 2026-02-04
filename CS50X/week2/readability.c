//
// Created by Rob Ross on 1/19/26.
//
// Coleman-Liau index formula for determining the reading level of a text:
// index = 0.0588 * L - 0.296 * S - 15.8
// L is the average number of letters per 100 words in text
// S is the average number of sentences per 100 words in text
//
// to make: clang readability.c ../cs50.c  -o readability

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include "../cs50.h"

int analyze_text(string text) {
    int score = 0;
    int letter_count = 0;
    int word_count = 0;
    int sentence_count = 0;

    int char_index = 0;
    bool in_word = false;

    while (text[char_index] != '\0') {
        if ( isspace(text[char_index]) ) {
            if (in_word) {
                word_count++;
                in_word = false;
            }
            else if (isalnum(text[char_index + 1])) {
                in_word = true;
            }
        }
        else if (isalnum(text[char_index])) {
            in_word = true;
            letter_count++;
        }
        else if (text[char_index] == '.' || text[char_index] == '!' || text[char_index] == '?') {
            sentence_count++;
            word_count++;
            in_word = false;
        }
        char_index++;
    }
    const float L  = (1.0 * letter_count / word_count) * 100.0;
    const float S  = (1.0 * sentence_count / word_count) * 100.0;
    const float float_score = 0.0588 * L - 0.296 * S - 15.8;
    printf("Float score: %f, L=%f, S=%f\n", float_score, L, S);
    score = round(float_score);
    printf("analyze_text: score: %i, letter_count: %i, word_count: %i, sentence_count: %i\n", score, letter_count, word_count, sentence_count);

    return score;
}

int main(void) {
    string text = get_string("text to score: ");
    int score = analyze_text(text);
    if (score < 1) {
        printf("Before Grade 1\n");
    }
    else if (score >= 16) {
        printf("Grade 16+\n");
    }
    else {
        printf("Grade %i\n", score);
    }
}