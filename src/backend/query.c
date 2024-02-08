
#define JSMN_HEADER
#include "query.h"
#include "json_stringify.h"
#include "UTIL/jsmn.h"
#include "UTIL/jsmn_helper.h"
#include "UTIL/util.h"
#include "UTIL/string.h"

void query_init(query_t *query){
    query->kind = QUERY_KIND_UNRECOGNIZED;
    query->infrastructure = NULL;
    query->filename = NULL;
    query->code = NULL;
    query->warnings = true;
    query->features = QUERY_FEATURE_NONE;
}

void query_free(query_t *query){
    free(query->filename);
    free(query->infrastructure);
    free(query->code);
}

successful_t query_parse(weak_cstr_t json, query_t *out_query, strong_cstr_t *out_error){
    jsmnh_obj_ctx_t ctx;
    if(jsmnh_obj_ctx_easy_init(&ctx, json, strlen(json))){
        if(out_error) *out_error = strclone("Failed to parse JSON structure");
        goto failure;
    }

    query_init(out_query);

    for(length_t section = 0; section != ctx.total_sections; section++){
        // Get next key inside request
        if(jsmnh_obj_ctx_read_key(&ctx)){
            if(out_error) *out_error = strclone("Failed to parse JSON key");
            goto failure;
        }

        if(jsmnh_obj_ctx_eq(&ctx, "query")){
            // "query" : "..."
            if(!jsmnh_obj_ctx_get_fixed_string(&ctx, ctx.value.content, ctx.value.capacity)){
                *out_error = mallocandsprintf("Expected string value for 'query'");
                goto failure;
            }

            if(!query_set_kind_by_name(out_query, ctx.value.content)){
                if(out_error){
                    strong_cstr_t escaped = json_stringify_string(ctx.value.content);
                    *out_error = mallocandsprintf("Unrecognized query kind %s", escaped);
                    free(escaped);
                }
                goto failure;
            }
        } else if(jsmnh_obj_ctx_eq(&ctx, "infrastructure")){
            // "infrastructure" : "..."
            if(!jsmnh_obj_ctx_get_variable_string(&ctx, &out_query->infrastructure)){
                *out_error = mallocandsprintf("Expected string value for '%s'", ctx.value.content);
                goto failure;
            }
        } else if(jsmnh_obj_ctx_eq(&ctx, "filename")){
            // "filename" : "..."
            if(!jsmnh_obj_ctx_get_variable_string(&ctx, &out_query->filename)){
                *out_error = mallocandsprintf("Expected string value for '%s'", ctx.value.content);
                goto failure;
            }
        } else if(jsmnh_obj_ctx_eq(&ctx, "code")){
            // "code" : "..."

            strong_cstr_t escaped;
            if(!jsmnh_obj_ctx_get_variable_string(&ctx, &escaped)){
                *out_error = mallocandsprintf("Expected string value for '%s'", ctx.value.content);
                goto failure;
            }

            out_query->code = unescape_code_string(escaped);
            free(escaped);
        } else if(jsmnh_obj_ctx_eq(&ctx, "warnings")){
            // "warnings" : ...

            bool warnings;
            if(!jsmnh_obj_ctx_get_boolean(&ctx, &out_query->warnings)){
                *out_error = mallocandsprintf("Expected boolean value for '%s'", ctx.value.content);
                goto failure;
            }
        } else if(jsmnh_obj_ctx_eq(&ctx, "features")){
            // "features" : ...

            query_features_t features = {0};

            if(!jsmnh_obj_ctx_get_array(&ctx)){
                *out_error = mallocandsprintf("Expected array value for '%s'", ctx.value.content);
                goto failure;
            }

            length_t count = ctx.tokens.tokens[ctx.token_index++].size;

            for(length_t i = 0; i < count; i++){
                strong_cstr_t content;
                if(!jsmnh_obj_ctx_get_variable_string(&ctx, &content)){
                    *out_error = mallocandsprintf("Expected a feature string in features array");
                    goto failure;
                }

                ctx.token_index++;

                if(streq(content, "include-arg-info")){
                    features |= QUERY_FEATURE_INCLUDE_ARG_INFO;
                } else if(streq(content, "include-calls")){
                    features |= QUERY_FEATURE_INCLUDE_CALLS;
                } else {
                    *out_error = mallocandsprintf("Unsupported feature '%s'", content);
                    free(content);
                    goto failure;
                }

                free(content);
            }

            out_query->features = features;
        } else {
            // "???" : "???"
            if(out_error) *out_error = mallocandsprintf("Unrecognized key '%s'", ctx.value.content);
            goto failure;
        }

        jsmnh_obj_ctx_blind_advance(&ctx);
    }

    jsmnh_obj_ctx_free(&ctx);
    return true;

failure:
    jsmnh_obj_ctx_free(&ctx);
    query_free(out_query);
    return false;
}

successful_t query_set_kind_by_name(query_t *out_query, weak_cstr_t kind_name){
    if(streq(kind_name, "validate")){
        out_query->kind = QUERY_KIND_VALIDATE;
        return true;
    }

    if(streq(kind_name, "ast")){
        out_query->kind = QUERY_KIND_AST;
        return true;
    }

    return false;
}

strong_cstr_t unescape_code_string(weak_cstr_t escaped){
    // From: "    \nprint(\"Isaac says \\\"Hello\\\"\"   "
    
    // \n -> newline
    // \" -> quote
    // \\ -> backslash

    strong_cstr_t result = NULL;
    length_t length = 0;
    length_t capacity = 0;

    char *s = escaped;

    while(*s){
        expand((void**) &result, sizeof(char), length, &capacity, 1, 4096);

        if(*s != '\\'){
            result[length++] = *(s++);
            continue;
        }

        switch(*(++s)){
        case 'n':  result[length++] = '\n'; break;
        case 't':  result[length++] = '\t'; break;
        case '"':  result[length++] = '"';  break;
        case '\\': result[length++] = '\\'; break;
        case '\'': result[length++] = '\''; break;
        case 'e':  result[length++] = 0x1B; break;
        default:
            printf("AdeptInsightNodeJS internal error: unescape_code_string() got bad escape code %c\n", *s);
            s--;
        }

        s++;
    }

    expand((void**) &result, sizeof(char), length, &capacity, 2, 4096);
    result[length++] = '\n';
    result[length++] = '\0';
    return result;
}
