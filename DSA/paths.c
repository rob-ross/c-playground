// paths.c
//
// Copyright (c) Rob Ross 2026.
//
//
// Created 2026/05/14 20:53:17 PDT

//
// Created by Rob Ross on 5/14/26.
//

#include "paths.h"
#include <stdlib.h> // For malloc, free
#include <stdio.h> // For printf

// Global ROOM_GRAPH definition
int ROOM_GRAPH[NUM_ROOMS][RGINDEX_COUNT] = {
    { 0,  0,  0,  0,  0,  0,  0}, // Room 0
    { 0,  5,  2,  0,  0,  0,  0}, // Room 1
    { 0,  0,  0,  1,  0,  0,  0}, // Room 2
    { 3,  7,  4,  3,  3,  3,  0}, // Room 3
    { 0,  0,  0,  3,  0,  0,  0}, // Room 4
    { 1,  5,  7,  5,  5,  5,  0}, // Room 5
    { 6,  6,  6,  6,  6,  6,  0}, // Room 6
    { 3,  0,  8,  5,  0,  0,  0}, // Room 7
    { 8, 12,  8,  7,  8,  8,  0}, // Room 8
    {11, 13, 10,  0,  0,  0,  0}, // Room 9
    { 0, 14,  0,  9,  0,  0,  0}, // Room 10
    { 9,  6,  6,  6,  6,  6,  0}, // Room 11
    { 8, 16, 19,  0,  0,  0,  0}, // Room 12
    {13,  0,  0, 13,  0, 13,  0}, // Room 13
    {10,  0, 15, 17,  0, 18,  0}, // Room 14
    { 0,  0,  0, 14,  0, 19,  0}, // Room 15
    {12, 16, 16, 18, 16, 16,  0}, // Room 16
    {14,  0, 18,  0,  0,  0,  0}, // Room 17
    { 0,  0, 16, 17, 14,  0,  0}, // Room 18
    { 0, 12,  0,  0, 15,  0,  0}, // Room 19
};

// Helper function to get the string name of a direction
const char* get_direction_name(enum RoomGraphIndex direction) {
    switch (direction) {
        case RGINDEX_NORTH: return "North";
        case RGINDEX_SOUTH: return "South";
        case RGINDEX_EAST:  return "East";
        case RGINDEX_WEST:  return "West";
        case RGINDEX_UP:    return "Up";
        case RGINDEX_DOWN:  return "Down";
        default:            return "N/A"; // For RGINDEX_CONTENTS or -1 (start node)
    }
}

// Structure to store parent information for path reconstruction
typedef struct ParentInfo {
    int parent_room_index;
    enum RoomGraphIndex direction_from_parent; // Direction taken from parent to reach this room
} ParentInfo;

PathNode* find_shortest_path(int start_room, int end_room, int* out_path_length) {
    // Helper structure for BFS queue (local to this function)
    typedef struct QueueNode {
        int room_index;
    } QueueNode;

    // Queue implementation (simple array-based circular queue, local to this function)
    QueueNode queue[NUM_ROOMS];
    int front = -1, rear = -1;

    if (start_room < 0 || start_room >= NUM_ROOMS || end_room < 0 || end_room >= NUM_ROOMS) {
        *out_path_length = 0;
        return nullptr; // Invalid room indices
    }

    if (start_room == end_room) {
        *out_path_length = 0; // Path length is 0 edges
        return nullptr;      // No path nodes needed for a 0-length path
    }

    bool visited[NUM_ROOMS];
    ParentInfo parent[NUM_ROOMS];

    // Initialize visited array and parent info
    for (int i = 0; i < NUM_ROOMS; i++) {
        visited[i] = false;
        parent[i].parent_room_index = -1;
        parent[i].direction_from_parent = -1;
    }

    // Start BFS
    {
       //enqueue
        front = 0;
        rear++;
        queue[rear].room_index = start_room;
    }
    visited[start_room] = true;

    int current_room = -1;
    while ( !(front == -1) ) { // This is equivalent to !is_queue_empty()
        QueueNode current;

        // dequeue
        current = queue[front];
        front++;
        if (front > rear) { // Queue is empty
            front = rear = -1;
        }

        current_room = current.room_index;

        if (current_room == end_room) {
            break; // Found the shortest path
        }

        // Explore neighbors
        // Iterate through the direction indices (North, South, East, West, Up, Down)
        for (int i = RGINDEX_NORTH; i <= RGINDEX_DOWN; i++) {
            int neighbor_room = ROOM_GRAPH[current_room][i];

            // A zero means no edge, and we don't want to revisit already visited rooms
            if (neighbor_room != 0 && !visited[neighbor_room]) {
                visited[neighbor_room] = true;
                parent[neighbor_room].parent_room_index = current_room;
                parent[neighbor_room].direction_from_parent = (enum RoomGraphIndex)i; // Direction from current_room to neighbor_room

                {
                    // enqueue(neighbor_room);
                    if (front == -1 ) front = 0;
                    rear++;
                    queue[rear].room_index = neighbor_room;
                }

            }
        }
    }

    // If end_room was not reached
    if (current_room != end_room) {
        *out_path_length = 0;
        return nullptr;
    }

    // Reconstruct the path
    PathNode* path = nullptr;
    int path_nodes_count = 0; // This will count the number of nodes in the path
    int temp_path_indices[NUM_ROOMS]; // Temporary array to store path room indices in reverse
    enum RoomGraphIndex temp_path_directions[NUM_ROOMS]; // Temporary array to store directions in reverse

    int current_node_in_reconstruction = end_room;
    while (current_node_in_reconstruction != -1) {
        temp_path_indices[path_nodes_count] = current_node_in_reconstruction;
        temp_path_directions[path_nodes_count] = parent[current_node_in_reconstruction].direction_from_parent;
        current_node_in_reconstruction = parent[current_node_in_reconstruction].parent_room_index;
        path_nodes_count++;
    }

    // Set path_length to number of edges
    *out_path_length = path_nodes_count - 1;

    // Allocate memory for the path nodes (path_nodes_count elements)
    path = (PathNode*)malloc(sizeof(PathNode) * path_nodes_count);
    if (path == nullptr) {
        *out_path_length = 0; // Indicate allocation failure
        return nullptr;
    }

    // Copy path in correct order
    for (int i = 0; i < path_nodes_count; i++) {
        path[i].room_index = temp_path_indices[path_nodes_count - 1 - i];
        path[i].direction_from_prev = temp_path_directions[path_nodes_count - 1 - i];
    }
    // The first node in the path has no previous direction
    if (path_nodes_count > 0) {
        path[0].direction_from_prev = -1;
    }

    return path;
}

// --- Implementation for find_longest_path_visiting_max_nodes ---

// Max path length heuristic: allow visiting each node up to 3 times to explore cycles
// without letting paths grow excessively long.
#define MAX_PATH_BUFFER_SIZE (NUM_ROOMS * 3)

// Recursive helper for find_longest_path_visiting_max_nodes
static void explore_path_recursive(
    int current_room,
    int end_room,
    PathNode* current_path_buffer, // Buffer to build current path
    int current_path_len,          // Current length of path in buffer (number of nodes)
    bool* unique_nodes_visited_on_current_path, // Tracks unique nodes visited on THIS path
    int current_unique_nodes_count,
    int* node_visit_counts_on_current_path, // Tracks total visits to a node on THIS path
    PathNode** best_path_ptr,      // Pointer to the best path found so far
    int* best_path_len,            // Length of the best path (number of nodes)
    int* best_unique_nodes_count   // Unique nodes in the best path
) {
    // Update best path if current path is better
    // Priority 1: Maximize unique nodes
    // Priority 2: Minimize total path length (number of edges)
    if (current_unique_nodes_count > *best_unique_nodes_count) {
        *best_unique_nodes_count = current_unique_nodes_count;
        *best_path_len = current_path_len;
        // Reallocate and copy best path
        if (*best_path_ptr != nullptr) free(*best_path_ptr);
        *best_path_ptr = (PathNode*)malloc(sizeof(PathNode) * current_path_len);
        if (*best_path_ptr == nullptr) { /* Handle error, e.g., exit or log */ return; }
        for (int i = 0; i < current_path_len; i++) {
            (*best_path_ptr)[i] = current_path_buffer[i];
        }
    } else if (current_unique_nodes_count == *best_unique_nodes_count) {
        // If unique nodes are equal, prefer shorter total path length
        if (current_path_len < *best_path_len) {
            *best_path_len = current_path_len;
            // Reallocate and copy best path
            if (*best_path_ptr != nullptr) free(*best_path_ptr);
            *best_path_ptr = (PathNode*)malloc(sizeof(PathNode) * current_path_len);
            if (*best_path_ptr == nullptr) { /* Handle error */ return; }
            for (int i = 0; i < current_path_len; i++) {
                (*best_path_ptr)[i] = current_path_buffer[i];
            }
        }
    }

    // Base case: Reached end_room
    if (current_room == end_room) {
        return; // Path ends here
    }

    // Pruning: If current path is already too long, stop exploring this branch.
    // This prevents infinite recursion in cycles and limits search space.
    if (current_path_len >= MAX_PATH_BUFFER_SIZE -1) { // -1 to leave space for next node
        return;
    }

    // Explore neighbors
    for (int i = RGINDEX_NORTH; i <= RGINDEX_DOWN; i++) {
        int next_room = ROOM_GRAPH[current_room][i];

        if (next_room != 0) { // If there's an edge
            // Heuristic to prevent excessive revisits to a single node in a path segment.
            // Allow visiting a node up to 2 times. More than that usually means
            // we're just looping without adding new unique nodes.
            // Exception: if next_room is the end_room, we can always visit it to complete a path.
            if (node_visit_counts_on_current_path[next_room] >= 2 && next_room != end_room) {
                continue;
            }

            // Add to current path
            current_path_buffer[current_path_len].room_index = next_room;
            current_path_buffer[current_path_len].direction_from_prev = (enum RoomGraphIndex)i;

            bool was_unique = !unique_nodes_visited_on_current_path[next_room];
            if (was_unique) {
                unique_nodes_visited_on_current_path[next_room] = true;
                current_unique_nodes_count++;
            }
            node_visit_counts_on_current_path[next_room]++;

            explore_path_recursive(
                next_room,
                end_room,
                current_path_buffer,
                current_path_len + 1,
                unique_nodes_visited_on_current_path,
                current_unique_nodes_count,
                node_visit_counts_on_current_path,
                best_path_ptr,
                best_path_len,
                best_unique_nodes_count
            );

            // Backtrack: revert changes made for this recursive call
            node_visit_counts_on_current_path[next_room]--;
            if (was_unique) {
                unique_nodes_visited_on_current_path[next_room] = false;
                current_unique_nodes_count--;
            }
            // No need to explicitly remove from current_path_buffer; it will be overwritten
            // or its length will be implicitly reduced by current_path_len in the next iteration.
        }
    }
}


PathNode* find_shortest_path_visiting_max_nodes(int start_room, int end_room, int* out_path_length, int* out_unique_nodes_visited) {
    if (start_room < 0 || start_room >= NUM_ROOMS || end_room < 0 || end_room >= NUM_ROOMS) {
        *out_path_length = 0;
        *out_unique_nodes_visited = 0;
        return nullptr;
    }

    // Handle start_room == end_room case
    if (start_room == end_room) {
        *out_path_length = 0;
        *out_unique_nodes_visited = 1; // One unique node visited (the start/end room)
        // If a path of PathNode is desired even for 0 edges, it would need to allocate a single PathNode for the start room.
        // For consistency with find_shortest_path, 0 edges means nullptr.
        return nullptr;
    }

    // Initialize best path variables
    PathNode* best_path = nullptr;
    int best_path_nodes_count = 0; // Number of nodes in the best path
    int best_unique_nodes_count = 0;

    // Initialize current path variables for recursion
    PathNode current_path_buffer[MAX_PATH_BUFFER_SIZE];
    bool unique_nodes_visited_on_current_path[NUM_ROOMS];
    int node_visit_counts_on_current_path[NUM_ROOMS];

    // Initialize for the first call
    for (int i = 0; i < NUM_ROOMS; i++) {
        unique_nodes_visited_on_current_path[i] = false;
        node_visit_counts_on_current_path[i] = 0;
    }

    // Start node setup for the initial call
    current_path_buffer[0].room_index = start_room;
    current_path_buffer[0].direction_from_prev = -1; // No direction to reach the start node

    unique_nodes_visited_on_current_path[start_room] = true;
    node_visit_counts_on_current_path[start_room]++;

    explore_path_recursive(
        start_room,
        end_room,
        current_path_buffer,
        1, // Initial path length is 1 (just the start node)
        unique_nodes_visited_on_current_path,
        1, // 1 unique node (start_room)
        node_visit_counts_on_current_path,
        &best_path,
        &best_path_nodes_count,
        &best_unique_nodes_count
    );

    // Output results
    *out_path_length = best_path_nodes_count > 0 ? best_path_nodes_count - 1 : 0; // Convert nodes count to edges
    *out_unique_nodes_visited = best_unique_nodes_count;

    return best_path;
}


int main(void) {

    printf("--- Shortest Path Tests ---\n\n");

    // Test case 1: Path from START_ROOM to END_ROOM (3 to 6)
    int start_node_1 = START_ROOM; // 3
    int end_node_1 = END_ROOM;     // 6

    int path_len_1 = 0;
    PathNode* path_1 = find_shortest_path(start_node_1, end_node_1, &path_len_1);

    if (path_1 != nullptr) {
        printf("Shortest path from Room %d to Room %d (length %d edges):\n", start_node_1, end_node_1, path_len_1);
        // Iterate through nodes, printing the direction *from* the current node to the next
        for (int i = 0; i < path_len_1 + 1; i++) { // path_len_1 + 1 is the number of nodes
            printf("  Room %d", path_1[i].room_index);
            if (i < path_len_1) { // If it's not the last node, print the direction to the next node
                printf(" %s", get_direction_name(path_1[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_1);
    } else {
        printf("No path found from Room %d to Room %d (length %d edges).\n", start_node_1, end_node_1, path_len_1);
    }
    printf("\n");

    // Test case 2: Start and end are the same (Room 5 to Room 5)
    int start_node_2 = 5;
    int end_node_2 = 5;
    int path_len_2 = 0;
    PathNode* path_2 = find_shortest_path(start_node_2, end_node_2, &path_len_2);
    if (path_2 != nullptr) {
        printf("Shortest path from Room %d to Room %d (length %d edges):\n", start_node_2, end_node_2, path_len_2);
        for (int i = 0; i < path_len_2 + 1; i++) {
            printf("  Room %d", path_2[i].room_index);
            if (i < path_len_2) {
                printf(" %s", get_direction_name(path_2[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_2);
    } else {
        printf("No path found from Room %d to Room %d (length %d edges).\n", start_node_2, end_node_2, path_len_2);
        // For 0-length path, we can still print the start room if desired
        if (path_len_2 == 0 && start_node_2 == end_node_2) {
            printf("  (Start and end are the same: Room %d)\n", start_node_2);
        }
    }
    printf("\n");

    // Test case 3: No path exists (e.g., from Room 13 to Room 1 )
    int start_node_3 = 13;
    int end_node_3 = 1;
    int path_len_3 = 0;
    PathNode* path_3 = find_shortest_path(start_node_3, end_node_3, &path_len_3);
    if (path_3 != nullptr) {
        printf("Shortest path from Room %d to Room %d (length %d edges):\n", start_node_3, end_node_3, path_len_3);
        for (int i = 0; i < path_len_3 + 1; i++) {
            printf("  Room %d", path_3[i].room_index);
            if (i < path_len_3) {
                printf(" %s", get_direction_name(path_3[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_3);
    } else {
        printf("No path found from Room %d to Room %d (length %d edges).\n", start_node_3, end_node_3, path_len_3);
    }
    printf("\n");

    // Test case 4: Path from Room 1 to Room 7
    int start_node_4 = 1;
    int end_node_4 = 7;
    int path_len_4 = 0;
    PathNode* path_4 = find_shortest_path(start_node_4, end_node_4, &path_len_4);
    if (path_4 != nullptr) {
        printf("Shortest path from Room %d to Room %d (length %d edges):\n", start_node_4, end_node_4, path_len_4);
        for (int i = 0; i < path_len_4 + 1; i++) {
            printf("  Room %d", path_4[i].room_index);
            if (i < path_len_4) {
                printf(" %s", get_direction_name(path_4[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_4);
    } else {
        printf("No path found from Room %d to Room %d (length %d edges).\n", start_node_4, end_node_4, path_len_4);
    }
    printf("\n");

    printf("--- Longest Path (Max Unique Nodes) Tests ---\n\n");

    // Test case 5: Longest path from START_ROOM to END_ROOM (3 to 6)
    int start_node_5 = START_ROOM; // 3
    int end_node_5 = END_ROOM;     // 6
    int path_len_5 = 0;
    int unique_nodes_5 = 0;
    PathNode* path_5 = find_shortest_path_visiting_max_nodes(start_node_5, end_node_5, &path_len_5, &unique_nodes_5);

    if (path_5 != nullptr) {
        printf("Longest path (max unique nodes) from Room %d to Room %d:\n", start_node_5, end_node_5);
        printf("  Total Edges: %d, Unique Nodes: %d\n", path_len_5, unique_nodes_5);
        for (int i = 0; i < path_len_5 + 1; i++) {
            printf("  Room %d", path_5[i].room_index);
            if (i < path_len_5) {
                printf(" %s", get_direction_name(path_5[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_5);
    } else {
        printf("No longest path (max unique nodes) found from Room %d to Room %d (length %d edges, %d unique nodes).\n", start_node_5, end_node_5, path_len_5, unique_nodes_5);
        if (start_node_5 == end_node_5 && unique_nodes_5 == 1) {
            printf("  (Start and end are the same: Room %d, 1 unique node)\n", start_node_5);
        }
    }
    printf("\n");

    // Test case 6: Longest path from Room 1 to Room 7
    int start_node_6 = 1;
    int end_node_6 = 7;
    int path_len_6 = 0;
    int unique_nodes_6 = 0;
    PathNode* path_6 = find_shortest_path_visiting_max_nodes(start_node_6, end_node_6, &path_len_6, &unique_nodes_6);

    if (path_6 != nullptr) {
        printf("Longest path (max unique nodes) from Room %d to Room %d:\n", start_node_6, end_node_6);
        printf("  Total Edges: %d, Unique Nodes: %d\n", path_len_6, unique_nodes_6);
        for (int i = 0; i < path_len_6 + 1; i++) {
            printf("  Room %d", path_6[i].room_index);
            if (i < path_len_6) {
                printf(" %s", get_direction_name(path_6[i+1].direction_from_prev));
            }
            printf("\n");
        }
        free(path_6);
    } else {
        printf("No longest path (max unique nodes) found from Room %d to Room %d (length %d edges, %d unique nodes).\n", start_node_6, end_node_6, path_len_6, unique_nodes_6);
    }
    printf("\n");


    return 0;
}