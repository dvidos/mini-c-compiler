#include <stdbool.h>


// code for unit tests should be enclosed in an "#ifdef UNIT_TESTS / #endif"
// this switch could also be provided from command line (gcc -DUNIT_TESTS)
#define INCLUDE_UNIT_TESTS 1


#ifdef INCLUDE_UNIT_TESTS

    #define assert(cond)            unit_tests_assert(cond, #cond, __FILE__, __LINE__)
    #define assertm(cond, message)  unit_tests_assert_msg(cond, message, __FILE__, __LINE__)

    void unit_tests_assert(bool cond, const char *expr, const char *filename, int line);
    void unit_tests_assert_msg(bool cond, const char *msg, const char *filename, int line);
    bool unit_tests_outcome(); // print results, return success or failure

#else 

    #define assert(cond)            (void)0
    #define assertm(cond, message)  (void)0

#endif
