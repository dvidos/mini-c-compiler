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

* then explore maybe algorithms or math, or graphics or other stuff

### todo

* make this directory more self-contained, with regards to tests


