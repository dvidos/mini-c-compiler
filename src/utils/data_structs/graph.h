#pragma once
#include <stdbool.h>
#include "../mempool.h"
#include "../data_types/str.h"
#include "list.h"
#include "iterator.h"


typedef struct graph graph;

graph *new_graph(mempool *mp);
bool   graph_is_empty(graph *g);
int    graph_nodes_count(graph *g);
bool   graph_add_node(graph *g, str *key, void *node_data);
bool   graph_add_vertex(graph *g, str *key_from, str *key_to, void *vertex_data);
void  *graph_get_node(graph *g, str *key);
list  *graph_get_neighbors(graph *g, str *node_key);
bool   graph_is_acyclic(graph *g);
int    graph_number_of_trees(graph *g);
list *graph_topologically_sort(graph *g);
list *graph_find_shortest_path(graph *g, str *from_key, str *to_key);
iterator *graph_create_bfs_iterator(mempool *mp, graph *g, str *first_key);
iterator *graph_create_dfs_iterator(mempool *mp, graph *g, str *first_key);
iterator *get_nodes_iterator(mempool *mp, graph *g);
iterator *get_links_iterator(mempool *mp, graph *g, str *node_key);


#ifdef INCLUDE_UNIT_TESTS
void graph_unit_tests();
#endif
