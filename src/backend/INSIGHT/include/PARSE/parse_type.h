
#ifndef _ISAAC_PARSE_TYPE_H
#define _ISAAC_PARSE_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "AST/ast_type.h"
#include "LEX/token.h"
#include "PARSE/parse_ctx.h"
#include "UTIL/ground.h"

// ------------------ parse_type ------------------
// Parses a type into an abstract syntax tree form.
// NOTE: Returns SUCCESS on success
// NOTE: If failed, will set 'out_type' to empty type (all zeros)
//       This means that calling ast_type_free on 'out_type' after this function fails is optional.
errorcode_t parse_type(parse_ctx_t *ctx, ast_type_t *out_type);

// ------------------ parse_type_func ------------------
// Parses a function type into an abstract syntax tree
// type-element form. Primary called from 'parse_type'.
errorcode_t parse_type_func(parse_ctx_t *ctx, ast_elem_func_t *out_func_elem);

// ------------------ parse_can_type_start_with ------------------
// Returns whether an AST type can start with a token
// Won't consider '[' token unless 'allow_open_bracket' is specified as true
bool parse_can_type_start_with(tokenid_t id, bool allow_open_bracket);

#ifdef __cplusplus
}
#endif

#endif // _ISAAC_PARSE_TYPE_H
