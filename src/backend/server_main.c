
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "query.h"
#include "json_builder.h"
#include "json_builder_ex.h"
#include "json_stringify.h"

#include "ValidationQuery.h"
#include "ASTQuery.h"

extern strong_cstr_t server_main(weak_cstr_t query_json){
    json_builder_t builder;
    json_builder_init(&builder);

    query_t query;
    strong_cstr_t error_message = NULL;
    if(!query_parse(query_json, &query, &error_message)){
        json_build_string(&builder, error_message);
        free(error_message);
        return json_builder_finalize(&builder);
    }
    
    switch(query.kind){
    case QUERY_KIND_VALIDATE:
        handle_validation_query(&query, &builder);
        break;
    case QUERY_KIND_AST:
        handle_ast_query(&query, &builder);
        break;
    default:
        json_build_string(&builder, "Query kind is missing or unrecognized");
        goto cleanup_and_finalize;
    }

cleanup_and_finalize:
    query_free(&query);
    return json_builder_finalize(&builder);
}
