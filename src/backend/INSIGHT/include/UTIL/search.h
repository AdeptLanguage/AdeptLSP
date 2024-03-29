
#ifndef _ISAAC_SEARCH_H
#define _ISAAC_SEARCH_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    ================================= search.h =================================
    Module for performing special search algorithms
    ----------------------------------------------------------------------------
*/

#include "UTIL/ground.h"

// ---------------- binary_string_search ----------------
// Performs a binary search to find a string in an array
maybe_index_t binary_string_search(char *strings[], length_t string_count, const char *target);
maybe_index_t binary_string_search_const(const char *const strings[], length_t string_count, const char *target);

// ---------------- binary_int_search ----------------
// Performs a binary search to find an int in an array
maybe_index_t binary_int_search(const int ints[], length_t int_count, int target);

#ifdef __cplusplus
}
#endif

#endif // _ISAAC_SEARCH_H
