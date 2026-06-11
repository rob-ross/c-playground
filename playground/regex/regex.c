// regex.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/06/10 17:38:13 PDT

//
// Created by Rob Ross on 6/10/26.
//

// -----------------------------------------------------------------
//      REGULAR EXPRESSION STRINGS
// -----------------------------------------------------------------

// RSL: "Regex Start of Line"
#define RSL      "^"
#define WB_START "[[:<:]]"
#define WB_END   "[[:>:]]"
#define SP       "[[:space:]]*"
#define WS       "[\x20\x09\x0A\x0D]"
#define WS_star  "[\x20\x09\x0A\x0D]*"
#include <errno.h>
#include <stdio.h>
#include <_regex.h>

// JSON spec grammar for a `number`:
static const char * const REGEX_NUMBER = RSL WS_star "(-?(0|[1-9])[0-9]*(\\.[0-9]+)?([eE][-+]?[0-9]+)?)" WS_star;
static const char * const BAD_PATTERN = "?<>**";
static char err_buffer[1024];


int main(void) {
    char const * const patterns[] = {REGEX_NUMBER, BAD_PATTERN};
    constexpr int num_patterns = sizeof (patterns) / sizeof (char*);
    regex_t compiled[num_patterns];
    // 1. step one, compile the regex pattern string. REG_EXTENDED is standard for using the more modern regex features.
    //     REG_ENHANCED provides more compatability with perl and python regex
    //     REG_ICASE - case insensitive matches
    // see man regexec, man 7 re_format

    printf("COMPILE STEP:");
    for (int i = 0; i < num_patterns; ++i) {
        printf("\n");
        errno = 0;
        int result = regcomp( &compiled[i], patterns[i], REG_EXTENDED);
        // printf("errno is : %d\n", errno);  // regcomp does not use errno to report errors.

        if (result == 0) {
            // no compilation errors.
            printf("regex pattern compiled: %s\n", patterns[i]);

            // what happens with regerror if we use it when there's no error?
            regerror(result, &compiled[i], err_buffer, 1024);
            printf("no compile error, regerror reports: %s\n", err_buffer);
            //output: *** unknown regexp error code ***
        } else {
            // we can use regerror() to get a a human-readable, printable message.
            //does errno get upated?
            regerror(result, &compiled[i], err_buffer, 1024);
            printf("regex pattern error while compiling:\n %s : %s\n", err_buffer, patterns[i]);

            // if you OR (|) the error code with REG_ITOA, it returns the printable name of the error code, e.g. “REG_NOMATCH”
            regerror(result | REG_ITOA, &compiled[i], err_buffer, 1024);
            printf("    err code name: %s\n", err_buffer);

            //Similarly, REG_ATOI converts a name to a number:
            char const *err_name = "REG_BADPAT";

            regerror(REG_ATOI, &(regex_t){.re_endp = err_name}, err_buffer, 1024);
            printf("    err code number: %s\n", err_buffer);  // REG_BADPAT = 2.
        }
    }
    printf("\nMATCHING STEP:");
    char const * text_string = "-123.456e7  and then there was more text.";  // json number
    // Step 2. how to parse text using regexec(). returns 0 for success and the non-zero code REG_NOMATCH for failure.
    for (int i = 0; i < num_patterns; ++i) {
        printf("\n");
        constexpr size_t max_groups = 4;  // for example, to accomodate 4 capture groups
        // The first element (0) is the entire match, subsequent elements are sub capture groups
        regmatch_t pmatch[max_groups] = {}; // Assuming max_groups - 1 capture groups + full match

        const int eflags = 0;
        //  By default, the NUL-terminated string pointed to by string is considered to be the text of an entire line,
        //  minus any terminating newline.  The eflags argument is the bitwise OR of zero or more of the following flags:
        // REG_NOTBOL, REG_NOTEOL, REG_STARTEND
        int result = regexec(&compiled[i], text_string, 4, pmatch, eflags);

        if (result == 0) {
            // successful match
            printf("Successful match:\n  pattern: %s\n  text: %s\n", patterns[i], text_string);
            // here, pmatch[0] holds the indices of the substring in text_string that matched.
            for (int j = 0; j < 4; ++j) {
                auto substr_len = pmatch[j].rm_eo - pmatch[j].rm_so;
                printf("catpure group[%d] start:%lld, end:%lld is '%.*s'\n",
                    j, pmatch[j].rm_so, pmatch[j].rm_eo, (int)substr_len, text_string + pmatch[j].rm_so);
            }

        } else {
            regerror(result | REG_ITOA, &compiled[i], err_buffer, 1024);
            printf("No regex match: %s\n'%s'", err_buffer, text_string);

            regerror(result, &compiled[i], err_buffer, 1024);
            printf(": error: '%s'\n", err_buffer);

            if (result == REG_NOMATCH) {
                // typical result of a non-match

            }
        }

    }


    // free the resources used by the compiled patterns:
    for (int i = 0; i < num_patterns; ++i) {
        regfree(&compiled[i]);
    }



}