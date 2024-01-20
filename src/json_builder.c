
#include "UTIL/util.h"
#include "UTIL/string.h"
#include "UTIL/datatypes.h"
#include "json_builder.h"
#include "json_stringify.h"

void json_builder_init(json_builder_t *builder){
    builder->buffer = NULL;
    builder->length = 0;
    builder->capacity = 0;
}

void json_builder_free(json_builder_t *builder){
    free(builder->buffer);
}

void json_build_string(json_builder_t *builder, weak_cstr_t string){
    strong_cstr_t escaped = json_stringify_string(string);
    json_builder_append(builder, escaped);
    free(escaped);
}

void json_build_integer(json_builder_t *builder, long long integer){
    strong_cstr_t string = int64_to_string(integer, "");
    json_builder_append(builder, string);
    free(string);
}

void json_build_null(json_builder_t *builder){
    json_builder_append(builder, "null");
}

void json_build_next(json_builder_t *builder){
    json_builder_append(builder, ",");
}

void json_build_array_start(json_builder_t *builder){
    json_builder_append(builder, "[");
}

void json_build_array_next(json_builder_t *builder){
    json_builder_append(builder, ",");
}

void json_build_array_end(json_builder_t *builder){
    json_builder_append(builder, "]");
}

void json_build_object_start(json_builder_t *builder){
    json_builder_append(builder, "{");
}

void json_build_object_key(json_builder_t *builder, weak_cstr_t key){
    strong_cstr_t string = json_stringify_string(key);
    json_builder_append(builder, string);
    free(string);

    json_builder_append(builder, ":");
}

void json_build_object_next(json_builder_t *builder){
    json_builder_append(builder, ",");
}

void json_build_object_end(json_builder_t *builder){
    json_builder_append(builder, "}");
}

void json_builder_append(json_builder_t *builder, weak_cstr_t string){
    length_t string_length = strlen(string);
    expand((void**) &builder->buffer, sizeof(char), builder->length, &builder->capacity, string_length + 1, 2048);
    memcpy(&builder->buffer[builder->length], string, string_length + 1);
    builder->length += string_length;
}

void json_builder_append_escaped(json_builder_t *builder, weak_cstr_t string){
    if(string_needs_escaping(string, '\"')){
        strong_cstr_t escaped = string_to_escaped_string(string, strlen(string), '\"', false);
        json_builder_append(builder, escaped);
        free(escaped);
    } else {
        json_builder_append(builder, string);
    }
}

void json_builder_remove(json_builder_t *builder, length_t amount){
    builder->length -= amount;
}

strong_cstr_t json_builder_finalize(json_builder_t *builder){
    strong_cstr_t result = builder->buffer;
    json_builder_init(builder);
    return result;
}
