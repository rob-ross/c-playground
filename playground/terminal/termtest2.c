// termtest2.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/20 17:29:23 PDT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>

// A struct to hold the original terminal settings
static struct termios orig_termios;

/**
 * @brief Restores the original terminal settings.
 * This function is registered with atexit() to ensure the terminal
 * is cleaned up properly when the program exits.
 */
void disableRawMode(void) {
    // Restore the original settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        perror("tcsetattr");
    }
    // Switch back to "Numeric Keypad Mode" so the terminal behaves normally after exit
    printf("\033>");
    fflush(stdout);
    printf("Terminal mode restored.\n");
}

/**
 * @brief Enables raw, non-canonical terminal mode.
 * Turns off line buffering (ICANON) and character echoing (ECHO).
 */
void enableRawMode(void) {
    // Get the current terminal attributes
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }

    // Register the cleanup function to run on exit
    atexit(disableRawMode);

    // Create a copy of the original settings to modify
    struct termios raw = orig_termios;

    // Modify the flags:
    // ICANON: Turn off canonical mode (don't wait for Enter).
    // ECHO: Turn off echoing (don't print typed characters).
    // ISIG: Turn off signals (Ctrl+C won't terminate, Ctrl+Z won't suspend).
    // IEXTEN: Turn off extended input processing (Ctrl+V).
    raw.c_lflag &= ~(ICANON | ECHO | ISIG | IEXTEN);

    // IXON: Turn off software flow control (Ctrl+S / Ctrl+Q).
    // ICRNL: Turn off translation of CR (13) to NL (10) (Enter key).
    raw.c_iflag &= ~(IXON | ICRNL);

    // VMIN and VTIME control the behavior of read()
    // VMIN = 0, VTIME > 0: read() will wait for VTIME deciseconds. It returns
    // as soon as any data is available, or times out if no data arrives.
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; // 100ms timeout

    // Apply the new "raw" settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(1);
    }

    // Switch to "Application Keypad Mode"
    // This tells the terminal emulator to send escape sequences for keypad keys
    printf("\033=");
    fflush(stdout);
}

// make :
// cd /Users/robross/Documents/Development/CLionProjects/CS50x/playground/terminal
//  DEBUG:
// clang -std=c23 -o termtest2.out termtest2.c

int main(void) {
    enableRawMode();

    printf("Entering raw mode. Press 'q' to quit.\n");
    printf("Note: 'q' will exit, but Ctrl+C will be printed as raw input.\n");
    printf("Try modifiers: Ctrl+A, Alt+X (if configured), Arrows, Numpad.\n\n");

    for (;;) {
        char c = '\0';
        // read() will time out after 100ms if no key is pressed.
        // On timeout, it returns 0. On error, -1. On success, bytes read.
        if (read(STDIN_FILENO, &c, 1) != 1) {
            // In this mode, not reading a byte is normal (timeout).
            // We just continue the loop to wait for the next keypress.
            continue;
        }

        // An escape character usually indicates a multi-byte sequence follows.
        if (c == '\033') {
            char seq[3];

            // Try to read the rest of the sequence. If read() times out (returns != 1),
            // we know it was just a single ESC key press.
            if (read(STDIN_FILENO, &seq[0], 1) != 1) {
                printf("You pressed: ESC\r\n");
                continue;
            }
            if (read(STDIN_FILENO, &seq[1], 1) != 1) {
                printf("You pressed: Alt+%c (ESC %c)\r\n", seq[0], seq[0]);
                continue;
            }

            // We have a 3-character sequence.
            if (seq[0] == '[') { // ANSI escape sequences (Arrows, etc.)
                switch (seq[1]) {
                    case 'A': printf("You pressed: Arrow Up\r\n"); break;
                    case 'B': printf("You pressed: Arrow Down\r\n"); break;
                    case 'C': printf("You pressed: Arrow Right\r\n"); break;
                    case 'D': printf("You pressed: Arrow Left\r\n"); break;
                    default: printf("Unrecognized ANSI sequence: ESC [ %c\r\n", seq[1]); break;
                }
            } else if (seq[0] == 'O') { // Application Keypad sequences
                switch (seq[1]) {
                    case 'x': printf("You pressed: Numpad 8\r\n"); break;
                    case 'y': printf("You pressed: Numpad 7\r\n"); break;
                    case 'w': printf("You pressed: Numpad 9\r\n"); break;
                    case 'l': printf("You pressed: Numpad Enter\r\n"); break;
                    case 'k': printf("You pressed: Numpad +\r\n"); break;
                    default: printf("Unrecognized Numpad sequence: ESC O %c\r\n", seq[1]); break;
                }
            } else {
                // If we got ESC [ Z, that is often Shift+Tab
                if (seq[0] == '[' && seq[1] == 'Z') {
                    printf("You pressed: Shift+Tab\r\n");
                } else {
                    printf("Unrecognized 3-char sequence: ESC %c %c\r\n", seq[0], seq[1]);
                }
            }
            continue; // Done processing this key press.
        }

        if (c == 'q') {
            printf("'q' pressed. Exiting.\r\n");
            break;
        }

        if (isprint(c)) {
            printf("You pressed: '%c' (ASCII: %d)\r\n", c, c);
        } else if (c < 32) {
            // Control characters are 0-31
            // ASCII 1-26 correspond to Ctrl+A through Ctrl+Z
            // ASCII 13 is Enter (if ICRNL is off)
            if (c == 13) printf("You pressed: Enter (ASCII 13)\r\n");
            else if (c == 9)  printf("You pressed: Tab (ASCII 9)\r\n");
            else if (c == 27) printf("You pressed: ESC\r\n"); // Should be caught above, but safety
            else printf("You pressed: Ctrl+%c (ASCII %d)\r\n", c + 64, c);
        } else if (c == 127) {
            printf("You pressed: Backspace (ASCII 127)\r\n");
        } else {
            // Don't print anything for other non-printable chars
            printf("Non-printable: %d\r\n", c);
        }
    }

    return 0;
}