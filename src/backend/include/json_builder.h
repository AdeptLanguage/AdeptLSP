
#ifndef _ISAAC_JSON_BUILDER_H
#define _ISAAC_JSON_BUILDER_H

#include "UTIL/ground.h"

// ---------------- json_builder_t ----------------
// Context for constructing JSON
typedef struct {
    strong_cstr_t buffer;
    length_t length;
    length_t capacity;
} json_builder_t;

// ---------------- json_builder_init ----------------
// Initializes a 'json_builder_t' object
void json_builder_init(json_builder_t *builder);

// ---------------- json_builder_free ----------------
// Frees memory allocated by a 'json_builder_t' object
void json_builder_free(json_builder_t *builder);

// ---------------- json_build_* ----------------
// Builds various parts of a JSON string
void json_build_string(json_builder_t *builder, weak_cstr_t string);
void json_build_integer(json_builder_t *builder, long long integer);
void json_build_null(json_builder_t *builder);
void json_build_next(json_builder_t *builder);

void json_build_array_start(json_builder_t *builder);
void json_build_array_next(json_builder_t *builder);
void json_build_array_end(json_builder_t *builder);

void json_build_object_start(json_builder_t *builder);
void json_build_object_key(json_builder_t *builder, weak_cstr_t key);
void json_build_object_next(json_builder_t *builder);
void json_build_object_end(json_builder_t *builder);

// ---------------- json_builder_append ----------------
// Append raw string to JSON builder
void json_builder_append(json_builder_t *builder, weak_cstr_t string);

// ---------------- json_builder_append_escaped ----------------
// Append raw string that will be escaped to JSON builder
void json_builder_append_escaped(json_builder_t *builder, weak_cstr_t string);

// ---------------- json_builder_append_escaped ----------------
// Removes the last 'amount' characters from the string being built.
// NOTE: Assumes operation is valid and there are enough characters
void json_builder_remove(json_builder_t *builder, length_t amount);

// ---------------- json_builder_finalize ----------------
// Returns the constructed serialized JSON
strong_cstr_t json_builder_finalize(json_builder_t *builder);

#endif // _ISAAC_JSON_BUILDER_H
