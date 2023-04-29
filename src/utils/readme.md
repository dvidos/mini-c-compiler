# utils

This folder contains some stand-alone data structures constructs,
to make C programs more elaborate.

The idea is to support the following structures:

* string - for better memory manipulation
* buffer - for binary data
* list - a dynamic array of pointers, many operations
* hashtable - a dynamic hash lookup array in O(1)
* iterator - an interface to iterate over arrays, hashtables, bstrees etc

* binary search tree (O(lg N))
* heaps (min/max in O(n))




### in the future

In the future, the following may be built, depending on projects:

* number (any nubmer, any size, any precision)
* variant, json object, aggregate
  * any combination of hashtables, arrays and variant values
  * ability to convert to and from yaml, json.
  * json schema to validate json objects
* memory pool (to allocate memory, then free the pool altogether)

* in-memory cache, a combination of hashmap and double linked list.
  * given a fixed amount of memory to allocate
  * when items are accessed, move them at the start of the linked list.
  * then we can always evict them, if the allocated memory is full.
  * pair this with possible callbacks for populating the cache and you have a very fast cached system!

* if we make graphs, we can have a `create_bfs_iterator()` and `create_dfs_iterator()`
to allow us to traverse the graphs easily!

* then explore maybe algorithms or math, or graphics or other stuff

### todo

* make this directory more self-contained, with regards to tests


