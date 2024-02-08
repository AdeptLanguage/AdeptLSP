

#ifndef _ISAAC_QUERY_H
#define _ISAAC_QUERY_H

#include "UTIL/ground.h"
#include "UTIL/trait.h"
#include "json_builder.h"

// ---------------- query_kind_t ----------------
// Kind of query
typedef enum {
    QUERY_KIND_UNRECOGNIZED,
    QUERY_KIND_VALIDATE,
    QUERY_KIND_AST
} query_kind_t;


// ---------------- query_features_t ----------------
typedef trait_t query_features_t;
#define QUERY_FEATURE_NONE TRAIT_NONE
#define QUERY_FEATURE_INCLUDE_ARG_INFO TRAIT_1
#define QUERY_FEATURE_INCLUDE_CALLS TRAIT_2

// ---------------- query_t ----------------
// A Query
typedef struct {
    query_kind_t kind;
    maybe_null_strong_cstr_t infrastructure;
    maybe_null_strong_cstr_t filename;
    maybe_null_strong_cstr_t code;
    bool warnings;
    query_features_t features;
} query_t;

// ---------------- query_t ----------------
// Manually initialized a query,
// This is performed automatically within 'query_parse'
void query_init(query_t *query);

// ---------------- query_t ----------------
// Frees memory allocated by query
void query_free(query_t *query);

// ---------------- query_parse ----------------
// Parses a query from JSON
// Returns whether query was successfully parsed
successful_t query_parse(weak_cstr_t json, query_t *out_query, strong_cstr_t *out_parse_error);

// ---------------- query_set_kind_by_name ----------------
// Sets query kind by name
successful_t query_set_kind_by_name(query_t *out_query, weak_cstr_t kind_name);

// ---------------- unescape_code_string ----------------
// Unescapes the code string given for 'code' parameter
strong_cstr_t unescape_code_string(weak_cstr_t escaped);

#endif // _ISAAC_QUERY_H
