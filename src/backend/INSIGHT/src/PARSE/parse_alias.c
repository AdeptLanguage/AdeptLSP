
#include <stdlib.h>

#include "AST/ast.h"
#include "AST/ast_type_lean.h"
#include "DRVR/compiler.h"
#include "LEX/token.h"
#include "PARSE/parse_alias.h"
#include "PARSE/parse_ctx.h"
#include "PARSE/parse_struct.h"
#include "PARSE/parse_type.h"
#include "PARSE/parse_util.h"
#include "TOKEN/token_data.h"
#include "UTIL/ground.h"
#include "UTIL/search.h"
#include "UTIL/string.h"
#include "UTIL/trait.h"

errorcode_t parse_alias(parse_ctx_t *ctx){
    // NOTE: Assumes 'alias' keyword

    ast_t *ast = ctx->ast;

    ast_type_t type;
    source_t source = ctx->tokenlist->sources[(*ctx->i)++];

    if(ctx->composite_association != NULL){
        compiler_panicf(ctx->compiler, source, "Cannot declare type alias within struct domain");
        return FAILURE;
    }

    maybe_null_strong_cstr_t name = NULL;
    strong_cstr_t *generics = NULL;
    length_t generics_length = 0;

    if(parse_generics(ctx, &generics, &generics_length)){
        goto failure;
    }
    
    if(ctx->compiler->traits & COMPILER_COLON_COLON && ctx->prename){
        name = ctx->prename;
        ctx->prename = NULL;
    } else {
        name = parse_take_word(ctx, "Expected alias name after 'alias' keyword");
    }
    
    // Ensure we have a name for the alias
    if(name == NULL) return FAILURE;
    
    // Prepend namespace name
    parse_prepend_namespace(ctx, &name);

    const char *invalid_names[] = {
        "Any", "AnyFixedArrayType", "AnyFuncPtrType", "AnyPtrType", "AnyStructType",
        "AnyType", "AnyTypeKind", "String", "StringOwnership", "bool", "byte", "double", "float", "int", "long", "ptr",
        "short", "successful", "ubyte", "uint", "ulong", "ushort", "usize", "void"
    };
    length_t invalid_names_length = sizeof(invalid_names) / sizeof(const char*);

    if(binary_string_search_const(invalid_names, invalid_names_length, name) != -1){
        compiler_panicf(ctx->compiler, source, "Reserved type name '%s' can't be used to create an alias", name);
        goto failure;
    }

    if(parse_eat(ctx, TOKEN_ASSIGN, "Expected '=' after alias name")
    || parse_ignore_newlines(ctx, "Expected type after '=' in alias")
    || parse_type(ctx, &type)){
        goto failure;
    }

    ast_add_alias(ast, name, type, generics, generics_length, TRAIT_NONE, source);
    return SUCCESS;

failure:
    free(name);
    free_strings(generics, generics_length);
    return FAILURE;
}
