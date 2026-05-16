// paths.h
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/05/14 20:53:17 PDT

//
// Initial experiments with path finding algorithms.
// Using the node graph from asimovian_aftermath.c
//

#ifndef CS50X_PATHS_H
#define CS50X_PATHS_H

enum RoomGraphIndex {
    RGINDEX_NORTH, RGINDEX_SOUTH, RGINDEX_EAST, RGINDEX_WEST, RGINDEX_UP, RGINDEX_DOWN, RGINDEX_CONTENTS,
    RGINDEX_COUNT
};

constexpr int NUM_ROOMS      = 20;
constexpr int START_ROOM     = 3;
constexpr int END_ROOM       = 6;
constexpr int POD_ROOM       = 11;
constexpr int RADIATION_ROOM = 13;


extern int ROOM_GRAPH[NUM_ROOMS][RGINDEX_COUNT];

// Structure to represent a node in the path
typedef struct PathNode {
    int room_index;
    enum RoomGraphIndex direction_from_prev; // Direction taken to reach this room from the previous one
} PathNode;

// Function to find the shortest path between two rooms
// Returns a dynamically allocated array of PathNode, and sets path_length.
// The caller is responsible for freeing the returned array.
PathNode* find_shortest_path(int start_room, int end_room, int* out_path_length);

// Function to find a path that maximizes the number of unique nodes visited
// between two rooms. If multiple paths visit the same maximum number of unique nodes,
// the one with the shortest total path length (number of edges) is returned.
// Returns a dynamically allocated array of PathNode, and sets path_length and unique_nodes_visited.
// The caller is responsible for freeing the returned array.
PathNode* find_shortest_path_visiting_max_nodes(int start_room, int end_room, int* out_path_length, int* out_unique_nodes_visited);

// Helper function to get the string name of a direction
const char* get_direction_name(enum RoomGraphIndex direction);


#endif //CS50X_PATHS_H