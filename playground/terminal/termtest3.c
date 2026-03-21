// termtest3.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/20 19:28:12 PDT

#include <stdio.h>
#include <stdlib.h>
#include <term.h>     // Core terminfo definitions
#include <curses.h>   // Often needed for macros like ERR, OK
#include <unistd.h>

// You must link with -lncurses (or -ltinfo)
// clang -o termtest3.out termtest3.c -lncurses

int main(void) {
    int err_ret;

    // 1. Initialize terminfo for the standard output (file descriptor 1)
    // passing NULL uses the TERM environment variable.
    if (setupterm(NULL, STDOUT_FILENO, &err_ret) == ERR) {
        fprintf(stderr, "Error initializing terminfo. Error code: %d\n", err_ret);
        return EXIT_FAILURE;
    }

    // 2. Retrieve capabilities (strings) using "capnames"
    // "clear" -> Clear screen
    // "cup"   -> Cursor Position (requires 2 args: row, col)
    // "bold"  -> Enter bold mode
    // "sgr0"  -> Exit all attributes (reset)
    char *clear_scr = tigetstr("clear");
    char *cursor_pos = tigetstr("cup");
    char *enter_bold = tigetstr("bold");
    char *exit_attr = tigetstr("sgr0");

    // Check if critical capabilities exist (they return (char*)-1 or NULL on failure)
    if (!clear_scr || !cursor_pos) {
        fprintf(stderr, "Terminal does not support necessary capabilities.\n");
        return EXIT_FAILURE;
    }

    // 3. Execute capabilities

    // Clear the screen
    putp(clear_scr);

    // Move cursor to Row 5, Column 10
    // Note: terminfo uses 0-based indexing for rows/cols! (ANSI is usually 1-based)
    // tparm substitutes arguments into the capability string.
    putp(tparm(cursor_pos, 5, 10));

    // Turn on bold
    if (enter_bold) putp(enter_bold);

    printf("Hello from Terminfo!");

    // Turn off attributes
    if (exit_attr) putp(exit_attr);

    // Move down a bit so the prompt doesn't overwrite us
    putp(tparm(cursor_pos, 7, 0));

    return EXIT_SUCCESS;
}