#include "unit_tests.h"

#include "mempool.h"
#include "instance.h"
#include "func_types.h"

#include "data_types/str.h"
#include "data_types/bin.h"

#include "data_structs/iterator.h"
#include "data_structs/list.h"
#include "data_structs/queue.h"
#include "data_structs/stack.h"
#include "data_structs/heap.h"
#include "data_structs/bstree.h"
#include "data_structs/hashtable.h"
#include "data_structs/graph.h"



#ifdef INCLUDE_UNIT_TESTS
#define utils_unit_tests()  \
    do { \
        mempool_unit_tests(); \
        instance_unit_tests(); \
        \
        str_unit_tests(); \
        bin_unit_tests(); \
        \
        list_unit_tests(); \
        bstree_unit_tests(); \
        queue_unit_tests(); \
        stack_unit_tests(); \
        heap_unit_tests(); \
        bstree_unit_tests(); \
        hashtable_unit_tests(); \
        graph_unit_tests(); \
        iterator_unit_tests(); \
    } while (0)
#endif