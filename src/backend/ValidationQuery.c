
#include "ValidationQuery.h"

#include "LEX/lex.h"
#include "DRVR/compiler.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "UTIL/string.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

void handle_validation_query(query_t *query, json_builder_t *builder){
    if(query->infrastructure == NULL){
        json_build_string(builder, "Validation query is missing field 'infrastructure'");
        return;
    }

    if(query->filename == NULL){
        json_build_string(builder, "Validation query is missing field 'filename'");
        return;
    }

    if(query->code == NULL){
        json_build_string(builder, "Validation query is missing field 'code'");
        return;
    }

    compiler_t compiler;
    compiler_init(&compiler);
    object_t *object = compiler_new_object(&compiler);

    object->filename = strclone(query->filename);
    object->full_filename = filename_absolute(object->filename);
    
    // Force object->full_filename to not be NULL
    if(object->full_filename == NULL) object->full_filename = strclone("");

    // Set compiler root
    compiler.root = strclone(query->infrastructure);

    if (!query->warnings) {
        compiler.traits |= COMPILER_NO_WARN;
        compiler.ignore |= COMPILER_IGNORE_ALL;
    }

    // NOTE: Passing ownership of 'code' to object instance!!!
    object->buffer = query->code;
    object->buffer_length = strlen(query->code);
    query->code = NULL;
    
    if(lex_buffer(&compiler, object))   goto store_and_cleanup;
    if(parse(&compiler, object))        goto store_and_cleanup;

    length_t i;

store_and_cleanup:
    json_build_array_start(builder);

    // Push warnings
    for(i = 0; i != compiler.warnings_length; i++){
        json_build_object_start(builder);

        json_build_object_key(builder, "kind");
        json_build_string(builder, "warning");
        json_build_next(builder);

        json_build_object_key(builder, "source");
        json_build_source(builder, &compiler, compiler.warnings[i].source);
        json_build_next(builder);

        json_build_object_key(builder, "message");
        json_build_string(builder, compiler.warnings[i].message);

        json_build_object_end(builder);
        if(i + 1 != compiler.warnings_length || compiler.error) json_build_next(builder);
    }

    if(compiler.error){
        json_build_object_start(builder);

        json_build_object_key(builder, "kind");
        json_build_string(builder, "error");
        json_build_next(builder);

        json_build_object_key(builder, "source");
        json_build_source(builder, &compiler, compiler.error->source);
        json_build_next(builder);

        json_build_object_key(builder, "message");
        json_build_string(builder, compiler.error->message);

        json_build_object_end(builder);
    }

    json_build_array_end(builder);

cleanup:
    compiler_free(&compiler);
    return;
}
