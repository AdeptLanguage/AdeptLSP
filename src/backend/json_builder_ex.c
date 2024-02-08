
#include "AST/ast_type.h"
#include "AST/TYPE/ast_type_identical.h"
#include "json_builder_ex.h"

void json_build_source(json_builder_t *builder, compiler_t *compiler, source_t source){
    json_build_object_start(builder);
    json_build_object_key(builder, "object");
    json_build_string(builder, compiler->objects[source.object_index]->full_filename);
    json_build_next(builder);
    json_build_object_key(builder, "index");
    json_build_integer(builder, source.index);
    json_build_next(builder);
    json_build_object_key(builder, "stride");
    json_build_integer(builder, source.stride);
    json_build_object_end(builder);
}

void json_build_func_definition(json_builder_t *builder, ast_func_t *func){
    json_builder_append(builder, "\"");

    json_builder_append_escaped(builder, func->name);

    json_build_func_parameters(builder, func->arg_names, func->arg_types, func->arg_type_traits, func->arg_defaults, func->arity, TRAIT_NONE, func->variadic_arg_name);
    json_builder_append(builder, " ");

    strong_cstr_t s = ast_type_str(&func->return_type);
    json_builder_append_escaped(builder, s);
    free(s);

    json_builder_append(builder, "\"");
}

void json_build_func_parameters(
    json_builder_t *builder,
    weak_cstr_t *arg_names,
    ast_type_t *arg_types,
    trait_t *arg_type_traits,
    ast_expr_t **arg_defaults,
    length_t arity,
    trait_t traits,
    maybe_null_weak_cstr_t variadic_arg_name
){
    json_builder_append(builder, "(");

    for(length_t i = 0; i != arity; i++){
        bool is_last = i + 1 == arity;

        if(arg_names){
            while(!is_last && ast_types_identical(&arg_types[i], &arg_types[i + 1])){
                json_builder_append_escaped(builder, arg_names[i]);
                if(arg_defaults && arg_defaults[i]) json_builder_append(builder, "?");
                json_builder_append(builder, ", ");
                is_last = ++i + 1 == arity;
            }

            json_builder_append_escaped(builder, arg_names[i]);
            if(arg_defaults && arg_defaults[i]) json_builder_append(builder, "?");
            json_builder_append(builder, " ");
        }

        if(arg_type_traits && arg_type_traits[i] & AST_FUNC_ARG_TYPE_TRAIT_POD){
            json_builder_append(builder, "POD ");
        }

        strong_cstr_t s = ast_type_str(&arg_types[i]);
        json_builder_append_escaped(builder, s);
        free(s);

        if(!is_last){
            json_builder_append(builder, ", ");
        } else if(traits & AST_FUNC_VARARG){
            json_builder_append(builder, ", ...");
        } else if(traits & AST_FUNC_VARIADIC){
            json_builder_append(builder, ", ");
        }
    }

    if(traits & AST_FUNC_VARIADIC){
        json_builder_append_escaped(builder, variadic_arg_name);
        json_builder_append(builder, " ...");
    }

    json_builder_append(builder, ")");
}

void json_build_composite_definition(json_builder_t *builder, ast_composite_t *composite){
    json_builder_append(builder, "\"");

    if(composite->is_class){
        json_builder_append(builder, "class ");
    } else if(ast_layout_is_simple_struct(&composite->layout)){
        json_builder_append(builder, "struct ");
    } else if(ast_layout_is_simple_struct(&composite->layout)){
        json_builder_append(builder, "union ");
    } else {
        json_builder_append(builder, "struct ");
    }

    if(composite->is_polymorphic){
        json_builder_append(builder, "<");
        ast_poly_composite_t *poly_composite = (ast_poly_composite_t*) composite;

        for(length_t i = 0; i < poly_composite->generics_length; i++){
            json_builder_append(builder, "$");
            json_builder_append_escaped(builder, poly_composite->generics[i]);
            json_builder_append(builder, ", ");
        }

        if(poly_composite->generics_length) json_builder_remove(builder, 2); // Remove trailing ', '
        
        json_builder_append(builder, "> ");
    }
    
    json_builder_append_escaped(builder, composite->name);    
    json_builder_append(builder, " (");

    if(ast_layout_is_simple_struct(&composite->layout) || ast_layout_is_simple_struct(&composite->layout)){
        ast_layout_t *layout = &composite->layout;
        ast_field_map_t *field_map = &layout->field_map;
        
        for(length_t i = 0; i != field_map->arrows_length; i++){
            ast_field_arrow_t *arrow = &field_map->arrows[i];
            
            json_builder_append_escaped(builder, arrow->name);
            json_builder_append(builder, " ");

            ast_type_t *field_type = ast_layout_skeleton_get_type(&layout->skeleton, arrow->endpoint);

            if(field_type == NULL){
                json_builder_append(builder, "<unknown type>");
                continue;
            } else {
                char *s = ast_type_str(field_type);
                json_builder_append_escaped(builder, s);
                free(s);
            }
            
            if(i + 1 < field_map->arrows_length){
                json_builder_append(builder, ", ");
            }
        }
    } else {
        json_builder_append(builder, "<complex composite layout>");    
    }

    json_builder_append(builder, ")\"");
}
