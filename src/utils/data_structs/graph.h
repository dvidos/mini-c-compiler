#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"
#include "../data_types/str.h"
#include "iterator.h"

typedef struct graph graph;

graph *new_graph(mempool *mp);
int    graph_length(graph *g);
bool   graph_empty(graph *g);
bool   graph_add(graph *g, str *key, void *node_data);
bool   graph_add_link(graph *g, str *key_from, str *key_to, void *vertex_data);
void  *graph_get(graph *g, str *key);
llist *graph_get_neighbors(graph *g, str *node_key);
bool   graph_acyclic(graph *g);
int    graph_number_of_trees(graph *g);
llist *graph_topological_sort(graph *g);
llist *graph_shortest_path(graph *g, str *from_key, str *to_key);
iterator *graph_create_bfs_iterator(graph *g, mempool *m, str *first_key);
iterator *graph_create_dfs_iterator(graph *g, mempool *m, str *first_key);


#ifdef INCLUDE_UNIT_TESTS
void graph_unit_tests();
#endif
