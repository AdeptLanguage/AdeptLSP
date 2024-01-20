
#ifndef _ISAAC_JSON_BUILDER_EX_H
#define _ISAAC_JSON_BUILDER_EX_H

#include "json_builder.h"

#include "UTIL/ground.h"
#include "DRVR/compiler.h"

void json_build_source(json_builder_t *builder, compiler_t *compiler, source_t source);
void json_build_func_definition(json_builder_t *builder, ast_func_t *func);

void json_build_func_parameters(
    json_builder_t *builder,
    weak_cstr_t *arg_names,
    ast_type_t *arg_types,
    trait_t *arg_type_traits,
    ast_expr_t **arg_defaults,
    length_t arity,
    trait_t traits,
    maybe_null_weak_cstr_t variadic_arg_name
);

void json_build_composite_definition(json_builder_t *builder, ast_composite_t *composite);

#endif // _ISAAC_JSON_BUILDER_EX_H
