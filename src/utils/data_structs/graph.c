#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "graph.h"

struct graph_vertex {
    str *remote_node_key;
    void *data; // e.g. weight or distance
};

struct graph_node {
    str *key;
    void *data;          // e.g. cities data
    hashtable *vertices; // item type is graph_vertex
};

struct graph {
    hashtable *nodes;
    mempool *mempool;
};


#ifdef INCLUDE_UNIT_TESTS
void graph_unit_tests() {
    // oh boy, this is going to be fun!
}
#endif

