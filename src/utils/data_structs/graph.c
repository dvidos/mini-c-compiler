#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "graph.h"


typedef struct graph graph;
typedef struct graph_node graph_node;

graph *new_graph(mempool *mp);
int    graph_length(graph *g);
bool   graph_empty(graph *g);
bool   graph_add_node(graph *g, str *key, void *node_data);
bool   graph_add_link(graph *g, str *key_from, str *key_to, void *vertex_data);
bool   graph_remove_node(graph *g, str *key_from, str *key_to, void *vertex_data);
bool   graph_remove_link(graph *g, str *key_from, str *key_to, void *vertex_data);
void  *graph_get_node(graph *g, str *key);
llist *graph_get_links_from(graph *g, str *node_key);
llist *graph_get_links_to(graph *g, str *node_key);
bool   graph_acyclic(graph *g);
int    graph_number_of_trees(graph *g);
llist *graph_topological_sort(graph *g);
llist *graph_shortest_path(graph *g, str *from_key, str *to_key);
iterator *graph_create_bfs_iterator(graph *g, mempool *m, str *first_key);
iterator *graph_create_dfs_iterator(graph *g, mempool *m, str *first_key);

#ifdef INCLUDE_UNIT_TESTS
void graph_unit_tests() {
    // oh boy, this is going to be fun!
}
#endif

