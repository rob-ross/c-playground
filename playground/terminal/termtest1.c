// termtest1.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/03/20 17:01:17 PDT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

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
    raw.c_lflag &= ~(ICANON | ECHO);

    // VMIN and VTIME control the behavior of read()
    // VMIN = 0, VTIME > 0: Timed read. Wait VTIME deciseconds for a char.
    // VMIN > 0, VTIME = 0: Blocking read. Wait indefinitely for VMIN chars.
    raw.c_cc[VMIN] = 1; // Wait for at least 1 character
    raw.c_cc[VTIME] = 0; // No timeout

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
// clang -std=c23 -o termtest1.out termtest1.c

int main(void) {
    enableRawMode();

    printf("Entering raw mode. Press 'q' to quit.\n");
    printf("Typed characters will not be echoed.\n\n");

    char c;
    // read() returns the number of bytes read.
    // In raw mode, it will return 1 after each key press.
    // It returns 0 on EOF (Ctrl+D) and -1 on error.
    while (read(STDIN_FILENO, &c, 1) == 1) {
        if (c >= ' ' && c < 127) { // Check if it's a printable character
            printf("You pressed: '%c' (ASCII: %d)\r\n", c, c);
        } else {
            printf("You pressed a non-printable character (ASCII: %d)\r\n", c);
        }

        if (c == 'q') {
            printf("'q' pressed. Exiting.\r\n");
            break;
        }
    }

    return 0;
}
