// file_reading.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/06/02 14:39:31 PDT


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <_string.h>
typedef const char* string;



typedef struct len_str_s {
    size_t len;
    string s;
} LenStr;

typedef struct value_pos_s {
    LenStr value;
    const char* file_name;
    int    start_char_pos;
    int    end_char_pos;
    int    line_num_start;
    int    col_start;
    int    line_num_end;
    int    col_end;
} ValuePos;

typedef int (*file_process_action)( FILE *fptr);

int get_next_file_chunk(FILE *fptr, int len, char buffer[static len]) {
    if (!fptr) {
        printf("get_next_file_chunk() called with nullptr.\n");
        return -1;
    }
    string s = fgets(buffer, len, fptr);
    if (s == nullptr) {
        if (ferror(fptr)) {
            fprintf(stderr, "\nError reading file during echo_file.\n");
            return errno;
        }
        return EOF;
    }

    return 0;
}

constexpr char QUOTATION_MARK = '"';

LenStr parsed_strings[100] = {}; // fixed buffer for example code. in prod code this would be a dynamic list.
ValuePos parsed_pos[100] = {};

int str_count = 0;
int parsed_pos_count = 0;

void display_parsed_strings( const int len, LenStr ls[static len] ) {
    for (int i = 0; i < len; ++i) {
        printf("len=%zd, str:'%s'\n", ls[i].len, ls[i].s);
    }
}

void display_value_positions( const int len, ValuePos vp[static len]) {
    for (int i = 0; i < len; ++i) {
        printf("str='%s', start:%d, end:%d, start[l:%d c:%d], end[l:%d c:%d]\n",
            vp[i].value.s,
            vp[i].start_char_pos, vp[i].end_char_pos ,
            vp[i].line_num_start, vp[i].col_start,
            vp[i].line_num_end, vp[i].col_end);
    }
}

void free_parsed_strings( const int len, LenStr ls[static len] ) {
    for (int i = 0; i < len; ++i) {
        free((void*)ls[i].s);
    }
}


int find_json_strings(FILE *fptr) {
    // iterate through the entire file one buffer chunk at a time.
    // if not state string_start, iterate buffer looking for double-quotes.
    //  if found, set state string_start, save start pos.
    //  if not found, get next file chunk, continue loop
    // if state is string_start, continue reading chunk until end of chunk or end of file or until '"" found
    char buffer[1024];
    bool in_string = false;
    
    // Dynamic buffer to accumulate the string
    size_t val_capacity = 128;
    size_t val_len = 0;
    char *val_buffer = malloc(val_capacity);


    if (!val_buffer) return ENOMEM;
    int line_count = 1;
    int col_count = 1;
    int char_count = 1;


    while ( get_next_file_chunk(fptr, sizeof(buffer), buffer) == 0  ) {
        int buffer_index = 0;
        while (buffer[buffer_index] != '\0') {
            char c = buffer[buffer_index++];
            char_count++;
            col_count++;
            if (c == '\n') {
                line_count++;
                col_count = 1;
            }

            if (!in_string) {
                if (c == QUOTATION_MARK) {
                    in_string = true;
                    val_len = 0; // Reset accumulator for new string
                    parsed_pos[parsed_pos_count++] =
                        (ValuePos){ .start_char_pos = char_count, .line_num_start = line_count, .col_start = col_count };
                }
            } else {
                if (c == QUOTATION_MARK) {
                    in_string = false;
                    val_buffer[val_len] = '\0';
                    printf("Found String: [%s]\n", val_buffer);
                    string s = strdup(val_buffer);
                    if (!s) {
                        free(val_buffer);
                        free_parsed_strings(str_count, parsed_strings);
                        return ENOMEM;
                    }
                    LenStr ls = {.len=val_len, .s = s};
                    parsed_strings[str_count++] = ls;
                    parsed_pos[parsed_pos_count - 1].end_char_pos = char_count;
                    parsed_pos[parsed_pos_count - 1].line_num_end = line_count;
                    parsed_pos[parsed_pos_count - 1].col_end = col_count;
                    parsed_pos[parsed_pos_count - 1].value = ls;


                } else {
                    // Check if we need to grow the accumulator
                    if (val_len + 1 >= val_capacity) {
                        val_capacity *= 2;
                        char *temp = realloc(val_buffer, val_capacity);
                        if (!temp) {
                            free(val_buffer);
                            free_parsed_strings(str_count, parsed_strings);
                            return ENOMEM;
                        }
                        val_buffer = temp;
                    }

                    // Handle the "MUST escape" rule
                    if (c < 32) {
                        fprintf(stderr, "Error: Unescaped control character (ASCII %d) in string.\n", c);
                        // Technically we should return an error here per JSON spec
                        c = ' '; 
                    }
                    val_buffer[val_len++] = c;
                }
            }
        }
    }

    if (in_string) {
        fprintf(stderr, "Error: reached EOF before string terminated.\n");
    }

    free(val_buffer);

    display_parsed_strings( str_count, parsed_strings);
    display_value_positions(parsed_pos_count, parsed_pos);

    free_parsed_strings(str_count, parsed_strings);
    return 0;
}

int echo_file( FILE *fptr) {
    printf("In echo file!\n");

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fptr) != nullptr ) {
        printf("%s",buffer);
    }

    if (ferror(fptr)) {
        fprintf(stderr, "\nError reading file during echo_file.\n");
        return errno;
    }

    putchar('\n');
    return 0;
}

// Algorithm pattern. Wraps file operations with safe open/close sections and error erporting.
// ensures stream is closed regardless of outcome
// file processing is delegated to the `function` argument function pointer.
int process_file(string file_name, file_process_action function) {
int process_file(string file_name, file_process_action function) {
    FILE *fptr = nullptr;

    fptr = fopen(file_name, "r");
    if (!fptr) {
        printf("fopen failed for %s, error:%d, ", file_name, errno);
        perror(" ");
        return errno;
    }

    printf("file opened: %s\n", file_name);
    int result = function(fptr);

    if (fclose(fptr) != 0) {
        printf("fclose failed for %s, error:%d, ", file_name, errno);
        perror(" ");
        return errno;
    }

    printf("file closed: %s\n", file_name);
    return result;
}



int main(void) {
    process_file("file_data_1.txt", echo_file);
    putchar('\n');
    process_file("file_data_2.txt", echo_file);

    printf("\n\n");
    process_file("find_strings.txt", find_json_strings);
}