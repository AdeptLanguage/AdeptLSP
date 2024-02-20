
#include "ASTQuery.h"

#include "LEX/lex.h"
#include "PARSE/parse.h"
#include "UTIL/util.h"
#include "UTIL/string.h"
#include "DRVR/compiler.h"
#include "UTIL/filename.h"
#include "UTIL/__insight_undo_overloads.h"

static void add_function_definition(json_builder_t *builder, compiler_t *compiler, ast_func_t *func, bool include_arg_info){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, func->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_build_func_definition(builder, func);
    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, func->source);
    json_build_object_next(builder);
    json_build_object_key(builder, "end");
    json_build_source(builder, compiler, func->end_source);
    
    if(include_arg_info){
        json_build_object_next(builder);
        json_build_object_key(builder, "args");
        json_build_array_start(builder);
        for(length_t i = 0; i < func->arity; i ++){
            if(i != 0){
                json_build_object_next(builder);
            }

            json_build_object_start(builder);
            if(func->arg_names && func->arg_names[i]){
                json_build_object_key(builder, "name");
                json_build_string(builder, func->arg_names[i]);
                json_build_object_next(builder);
            }
            json_build_object_key(builder, "type");
            strong_cstr_t typename = ast_type_str(&func->arg_types[i]);
            json_build_string(builder, typename);
            free(typename);
            
            if(func->arg_defaults && func->arg_defaults[i]){
                json_build_object_next(builder);
                json_build_object_key(builder, "defaultValue");
                strong_cstr_t default_value_str = ast_expr_str(func->arg_defaults[i]);
                json_build_string(builder, default_value_str);
                free(default_value_str);
            }

            json_build_object_end(builder);
        }
        json_build_array_end(builder);
    }

    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_function_alias_definition(json_builder_t *builder, compiler_t *compiler, ast_func_alias_t *falias){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, falias->from);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");

    json_builder_append(builder, "\"");
    json_builder_append(builder, "func alias ");
    json_builder_append_escaped(builder, falias->from);

    if(!falias->match_first_of_name){
        json_build_func_parameters(builder, NULL, falias->arg_types, NULL, NULL, falias->arity, falias->required_traits, NULL);
    }
    
    json_builder_append(builder, " => ");
    json_builder_append_escaped(builder, falias->to);

    json_builder_append(builder, "\"");

    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, falias->source);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_composite_definition(json_builder_t *builder, compiler_t *compiler, ast_composite_t *composite){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, composite->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_build_composite_definition(builder, composite);
    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, composite->source);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_enum_definition(json_builder_t *builder, compiler_t *compiler, ast_enum_t *enum_value){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, enum_value->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"enum ");
    json_builder_append_escaped(builder, enum_value->name);
    json_builder_append(builder, " (");
    for(length_t i = 0; i != enum_value->length; i++){
        json_builder_append_escaped(builder, enum_value->kinds[i]);
        json_builder_append(builder, ", ");
    }
    if(enum_value->length != 0) json_builder_remove(builder, 2); // Remove trailing ', '
    json_builder_append(builder, ")\"");
    json_build_object_next(builder);
    json_build_object_key(builder, "source");
    json_build_source(builder, compiler, enum_value->source);
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_alias_definition(json_builder_t *builder, ast_alias_t *alias){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, alias->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"alias ");

    if(alias->generics_length > 0){
        json_builder_append(builder, "<");

        for(length_t i = 0; i < alias->generics_length; i++){
            if(i != 0){
                json_builder_append(builder, ", ");
            }

            json_builder_append(builder, "$");
            json_builder_append_escaped(builder, alias->generics[i]);
        }

        json_builder_append(builder, "> ");
    }

    json_builder_append_escaped(builder, alias->name);
    json_builder_append(builder, " = ");

    strong_cstr_t typename = ast_type_str(&alias->type);
    json_builder_append_escaped(builder, typename);
    free(typename);

    json_builder_append(builder, "\"");
    json_build_object_end(builder);

    json_build_object_next(builder);
}

static void add_named_expression_definition(json_builder_t *builder, ast_named_expression_t *named_expression){
    json_build_object_start(builder);
    json_build_object_key(builder, "name");
    json_build_string(builder, named_expression->name);
    json_build_object_next(builder);
    json_build_object_key(builder, "definition");
    json_builder_append(builder, "\"define ");
    json_builder_append_escaped(builder, named_expression->name);
    json_builder_append(builder, " = ");

    strong_cstr_t value = ast_expr_str(named_expression->expression);
    json_builder_append_escaped(builder, value);
    free(value);
    json_builder_append(builder, "\"");
    json_build_object_end(builder);

    json_build_object_next(builder);
}

void build_ast(json_builder_t *builder, compiler_t *compiler, object_t *object, query_features_t features){
    ast_t *ast = &object->ast;

    json_build_object_start(builder);

    {
        json_build_object_key(builder, "functions");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->funcs_length; i++){
            add_function_definition(builder, compiler, &ast->funcs[i], features & QUERY_FEATURE_INCLUDE_ARG_INFO);
        }
        if(ast->funcs_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "function_aliases");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->func_aliases_length; i++){
            add_function_alias_definition(builder, compiler, &ast->func_aliases[i]);
        }
        if(ast->func_aliases_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "composites");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->composites_length; i++){
            add_composite_definition(builder, compiler, &ast->composites[i]);
        }
        for(length_t i = 0; i < ast->poly_composites_length; i++){
            add_composite_definition(builder, compiler, (ast_composite_t*) &ast->poly_composites[i]);
        }
        if(ast->composites_length || ast->poly_composites_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "enums");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->enums_length; i++){
            add_enum_definition(builder, compiler, &ast->enums[i]);
        }
        if(ast->enums_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "aliases");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->aliases_length; i++){
            add_alias_definition(builder, &ast->aliases[i]);
        }
        if(ast->aliases_length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "namedExpressions");
        json_build_array_start(builder);
        for(length_t i = 0; i < ast->named_expressions.length; i++){
            add_named_expression_definition(builder, &ast->named_expressions.expressions[i]);
        }

        if(ast->named_expressions.length) json_builder_remove(builder, 1); // Remove trailing ','
        json_build_array_end(builder);
    }
    json_build_object_end(builder);
}

void build_identifierTokens(json_builder_t *builder, object_t *object){
    tokenlist_t *tokenlist = &object->tokenlist;
    token_t *tokens = tokenlist->tokens;
    source_t *sources = tokenlist->sources;
    bool has_at_least_one_identifier_token = false;

    json_build_array_start(builder);

    for(length_t i = 0; i != tokenlist->length; i++){
        if(tokens[i].id != TOKEN_WORD) continue;

        if(has_at_least_one_identifier_token){
            json_build_object_next(builder);
        } else {
            has_at_least_one_identifier_token = true;
        }

        token_t *token = &tokens[i];
        source_t *source = &sources[i];

        json_build_object_start(builder);
        json_build_object_key(builder, "content");
        json_build_string(builder, (weak_cstr_t) token->data);
        json_build_object_next(builder);
        json_build_object_key(builder, "range");

        int start_line, start_character, end_line, end_character;
        lex_get_location(object->buffer, source->index, &start_line, &start_character);
        lex_get_location(object->buffer, source->index + source->stride, &end_line, &end_character);

        // zero-indexed
        start_line--;
        start_character--;
        end_line--;
        end_character--;

        json_build_object_start(builder);

        json_build_object_key(builder, "start");

        json_build_object_start(builder);
        json_build_object_key(builder, "line");
        json_build_integer(builder, start_line);
        json_build_object_next(builder);
        json_build_object_key(builder, "character");
        json_build_integer(builder, start_character);
        json_build_object_end(builder);
        json_build_object_next(builder);

        json_build_object_key(builder, "end");

        json_build_object_start(builder);
        json_build_object_key(builder, "line");
        json_build_integer(builder, end_line);
        json_build_object_next(builder);
        json_build_object_key(builder, "character");
        json_build_integer(builder, end_character);
        json_build_object_end(builder);
        
        json_build_object_end(builder);
        json_build_object_end(builder);
    }

    json_build_array_end(builder);
}

typedef struct {
    weak_cstr_t *names;
    length_t *counts;
    length_t length;
    length_t function_index;
} call_info_t;

static void build_calls_for_expressions(ast_expr_t **exprs, length_t length, call_info_t *call_info);
static void build_calls_for_expression_list(ast_expr_list_t *exprs, call_info_t *call_info);

static void build_calls_for_expression(ast_expr_t *expr, call_info_t *call_info){
    if(expr == NULL) return;

    maybe_null_weak_cstr_t name = NULL;

    switch(expr->id){
    case EXPR_BYTE:
    case EXPR_UBYTE:
    case EXPR_SHORT:
    case EXPR_USHORT:
    case EXPR_INT:
    case EXPR_UINT:
    case EXPR_LONG:
    case EXPR_ULONG:
    case EXPR_USIZE:
    case EXPR_FLOAT:
    case EXPR_DOUBLE:
    case EXPR_BOOLEAN:
    case EXPR_STR:
    case EXPR_CSTR:
    case EXPR_NULL:
    case EXPR_GENERIC_INT:
    case EXPR_GENERIC_FLOAT:
        break;
    case EXPR_ADD:
    case EXPR_SUBTRACT:
    case EXPR_MULTIPLY:
    case EXPR_DIVIDE:
    case EXPR_MODULUS:
    case EXPR_EQUALS:
    case EXPR_NOTEQUALS:
    case EXPR_GREATER:
    case EXPR_LESSER:
    case EXPR_GREATEREQ:
    case EXPR_LESSEREQ:
    case EXPR_AND:
    case EXPR_OR:
    case EXPR_BIT_AND:
    case EXPR_BIT_OR:
    case EXPR_BIT_XOR:
    case EXPR_BIT_LSHIFT:
    case EXPR_BIT_RSHIFT:
    case EXPR_BIT_LGC_LSHIFT:
    case EXPR_BIT_LGC_RSHIFT:
        build_calls_for_expression(((ast_expr_math_t*) expr)->a, call_info);
        build_calls_for_expression(((ast_expr_math_t*) expr)->b, call_info);
        break;
    case EXPR_NEGATE:
    case EXPR_ADDRESS:
    case EXPR_DEREFERENCE:
    case EXPR_SIZEOF_VALUE:
    case EXPR_PREINCREMENT:
    case EXPR_PREDECREMENT:
    case EXPR_POSTINCREMENT:
    case EXPR_POSTDECREMENT:
    case EXPR_TOGGLE:
    case EXPR_NOT:
    case EXPR_BIT_COMPLEMENT:
        build_calls_for_expression(((ast_expr_unary_t*) expr)->value, call_info);
        break;
    case EXPR_ARRAY_ACCESS:
    case EXPR_AT:
        build_calls_for_expression(((ast_expr_array_access_t*) expr)->value, call_info);
        break;
    case EXPR_SUPER:
        build_calls_for_expressions(((ast_expr_super_t*) expr)->args, ((ast_expr_super_t*) expr)->arity, call_info);
        break;
    case EXPR_VARIABLE:
        break;
    case EXPR_MEMBER:
        build_calls_for_expression(((ast_expr_member_t*) expr)->value, call_info);
        break;
    case EXPR_FUNC_ADDR:
        break;
    case EXPR_CAST:
        build_calls_for_expression(((ast_expr_cast_t*) expr)->from, call_info);
        break;
    case EXPR_SIZEOF:
    case EXPR_ALIGNOF:
        break;
    case EXPR_NEW: {
            ast_expr_t *amount = ((ast_expr_new_t*) expr)->amount;
            if(amount) build_calls_for_expression(amount, call_info);
        }
        break;
    case EXPR_NEW_CSTRING:
        break;
    case EXPR_ENUM_VALUE:
    case EXPR_GENERIC_ENUM_VALUE:
        break;
    case EXPR_STATIC_ARRAY:
    case EXPR_STATIC_STRUCT: {
            ast_expr_static_data_t *static_data = ((ast_expr_static_data_t*) expr);
            build_calls_for_expressions(static_data->values, static_data->length, call_info);
        }
        break;
    case EXPR_TYPEINFO:
        break;
    case EXPR_TERNARY:
        build_calls_for_expression(((ast_expr_ternary_t*) expr)->if_true, call_info);
        build_calls_for_expression(((ast_expr_ternary_t*) expr)->if_false, call_info);
        break;
    case EXPR_PHANTOM:
        break;
    case EXPR_VA_ARG:
        build_calls_for_expression(((ast_expr_va_arg_t*) expr)->va_list, call_info);
        break;
    case EXPR_INITLIST: {
            ast_expr_initlist_t *initlist = ((ast_expr_initlist_t*) expr);
            build_calls_for_expressions(initlist->elements, initlist->length, call_info);
        }
        break;
    case EXPR_POLYCOUNT:
        break;
    case EXPR_TYPENAMEOF:
        break;
    case EXPR_LLVM_ASM: {
            ast_expr_llvm_asm_t *llvm_asm = (ast_expr_llvm_asm_t*) expr;
            build_calls_for_expressions(llvm_asm->args, llvm_asm->arity, call_info);
        }
        break;
    case EXPR_EMBED:
        break;
    case EXPR_DECLARE:
    case EXPR_DECLAREUNDEF:
    case EXPR_ILDECLARE:
    case EXPR_ILDECLAREUNDEF: {
            ast_expr_declare_t *declare = (ast_expr_declare_t*) expr;
            if(declare->inputs.has) build_calls_for_expression_list(&declare->inputs.value, call_info);
            if(declare->value) build_calls_for_expression(declare->value, call_info);
        }
        break;
    case EXPR_ASSIGN:
    case EXPR_ADD_ASSIGN:
    case EXPR_SUBTRACT_ASSIGN:
    case EXPR_MULTIPLY_ASSIGN:
    case EXPR_DIVIDE_ASSIGN:
    case EXPR_MODULUS_ASSIGN:
    case EXPR_AND_ASSIGN:
    case EXPR_OR_ASSIGN:
    case EXPR_XOR_ASSIGN:
    case EXPR_LSHIFT_ASSIGN:
    case EXPR_RSHIFT_ASSIGN:
    case EXPR_LGC_LSHIFT_ASSIGN:
    case EXPR_LGC_RSHIFT_ASSIGN: {
            ast_expr_assign_t *assign = (ast_expr_assign_t*) expr;
            build_calls_for_expression(assign->destination, call_info);
            build_calls_for_expression(assign->value, call_info);
        }
        break;
    case EXPR_RETURN: {
            ast_expr_return_t *ret = (ast_expr_return_t*) expr;
            if(ret->value) build_calls_for_expression(ret->value, call_info);
            build_calls_for_expression_list(&ret->last_minute, call_info);
        }
        break;
    case EXPR_DELETE:
        build_calls_for_expression(((ast_expr_delete_t*) expr)->value, call_info);
        break;
    case EXPR_BREAK:
    case EXPR_CONTINUE:
    case EXPR_FALLTHROUGH:
    case EXPR_BREAK_TO:
    case EXPR_CONTINUE_TO:
        break;
    case EXPR_VA_START: {
            ast_expr_va_start_t *start = (ast_expr_va_start_t*) expr;
            build_calls_for_expression(start->value, call_info);
        }
        break;
    case EXPR_VA_END: {
            ast_expr_va_end_t *end = (ast_expr_va_end_t*) expr;
            build_calls_for_expression(end->value, call_info);
        }
        break;
    case EXPR_VA_COPY: {
            ast_expr_va_copy_t *copy = (ast_expr_va_copy_t*) expr;
            build_calls_for_expression(copy->src_value, call_info);
            build_calls_for_expression(copy->dest_value, call_info);
        }
        break;
    case EXPR_DECLARE_NAMED_EXPRESSION: {
            ast_expr_declare_named_expression_t *declare_named_expression = (ast_expr_declare_named_expression_t*) expr;
            build_calls_for_expression(declare_named_expression->named_expression.expression, call_info);
        }
        break;
    case EXPR_ASSERT: {
            ast_expr_assert_t *assertion = (ast_expr_assert_t*) expr;
            if(assertion->assertion) build_calls_for_expression(assertion->assertion, call_info);
            if(assertion->message) build_calls_for_expression(assertion->message, call_info);
        }
        break;
    case EXPR_CALL: {
            ast_expr_call_t *call = (ast_expr_call_t*) expr;
            name = call->name;
            build_calls_for_expressions(call->args, call->arity, call_info);
        }
        break;
    case EXPR_CALL_METHOD: {
            ast_expr_call_method_t *call_method = (ast_expr_call_method_t*) expr;
            name = call_method->name;
            build_calls_for_expressions(call_method->args, call_method->arity, call_info);
            build_calls_for_expression(call_method->value, call_info);
        }
        break;
    case EXPR_IF:
    case EXPR_UNLESS:
    case EXPR_WHILE:
    case EXPR_UNTIL:
        build_calls_for_expression_list(&((ast_expr_if_t*) expr)->statements, call_info);
        break;
    case EXPR_IFELSE:
    case EXPR_UNLESSELSE:
        build_calls_for_expression_list(&((ast_expr_ifelse_t*) expr)->statements, call_info);
        build_calls_for_expression_list(&((ast_expr_ifelse_t*) expr)->else_statements, call_info);
        break;
    case EXPR_WHILECONTINUE:
    case EXPR_UNTILBREAK:
        build_calls_for_expression_list(&((ast_expr_whilecontinue_t*) expr)->statements, call_info);
        break;
    case EXPR_CONDITIONLESS_BLOCK:
        build_calls_for_expression_list(&((ast_expr_conditionless_block_t*) expr)->statements, call_info);
        break;
    case EXPR_FOR: {
            ast_expr_for_t *for_loop = (ast_expr_for_t*) expr;
            build_calls_for_expression_list(&for_loop->before, call_info);
            build_calls_for_expression_list(&for_loop->after, call_info);
            build_calls_for_expression(for_loop->condition, call_info);
            build_calls_for_expression_list(&for_loop->statements, call_info);
        }
        break;
    case EXPR_EACH_IN: {
            ast_expr_each_in_t *each_in = (ast_expr_each_in_t*) expr;
            if(each_in->length) build_calls_for_expression(each_in->length, call_info);
            if(each_in->list) build_calls_for_expression(each_in->list, call_info);
            if(each_in->low_array) build_calls_for_expression(each_in->low_array, call_info);

            build_calls_for_expression_list(&each_in->statements, call_info);
        }
        break;
    case EXPR_REPEAT: {
            ast_expr_repeat_t *repeat = (ast_expr_repeat_t*) expr;
            if(repeat->limit) build_calls_for_expression(repeat->limit, call_info);
            build_calls_for_expression_list(&repeat->statements, call_info);
        }
        break;
    case EXPR_SWITCH: {
            ast_expr_switch_t *switch_stmt = (ast_expr_switch_t*) expr;

            build_calls_for_expression(switch_stmt->value, call_info);
            build_calls_for_expression_list(&switch_stmt->or_default, call_info);

            for(length_t i = 0; i < switch_stmt->cases.length; i++){
                ast_case_t *switch_case = &switch_stmt->cases.cases[i];
                build_calls_for_expression(switch_case->condition, call_info);
                build_calls_for_expression_list(&switch_case->statements, call_info);
            }
        }
        break;
    default:
        die("build_calls_for_expression() - Got unrecognized expression ID 0x%08X\n", expr->id);
        return;
    }

    if(name){
        strong_cstr_list_t dummy_list = {
            .items = call_info->names,
            .length = call_info->length,
            .capacity = call_info->length,
        };

        maybe_index_t index = strong_cstr_list_bsearch(&dummy_list, name);

        if(index >= 0)
            call_info->counts[index]++;
    }
}

static void build_calls_for_expressions(ast_expr_t **exprs, length_t length, call_info_t *call_info){
    if(exprs == NULL) return;

    for(length_t i = 0; i < length; i++){
        build_calls_for_expression(exprs[i], call_info);
    }
}

static void build_calls_for_expression_list(ast_expr_list_t *exprs, call_info_t *call_info){
    for(length_t i = 0; i < exprs->length; i++){
        build_calls_for_expression(exprs->expressions[i], call_info);
    }
}

static void build_calls(json_builder_t *builder, compiler_t *compiler, object_t *object, query_features_t features){
    ast_func_t *funcs = object->ast.funcs;
    length_t funcs_length = object->ast.funcs_length;

    call_info_t call_info = {
        .names = malloc(sizeof(weak_cstr_t) * funcs_length),
        .counts = NULL,
        .length = funcs_length,
        .function_index = 0,
    };

    for(length_t i = 0; i < funcs_length; i++){
        call_info.names[i] = funcs[i].name;
    }

    qsort(call_info.names, call_info.length, sizeof(weak_cstr_t), string_compare_for_qsort);

    for(length_t i = 1; i < call_info.length; i++){
        if(streq(call_info.names[i], call_info.names[i - 1])){
            memmove(&call_info.names[i], &call_info.names[i + 1], sizeof(weak_cstr_t) * (call_info.length - i - 1));
            call_info.length--;
            i--;
        }
    }
    call_info.counts = malloc(call_info.length * sizeof(length_t));

    json_build_array_start(builder);

    for(length_t i = 0; i < funcs_length; i++){
        memset(call_info.counts, 0, sizeof(length_t) * call_info.length);
        call_info.function_index = i;

        build_calls_for_expression_list(&funcs[i].statements, &call_info);

        if(i != 0){
            json_build_array_next(builder);
        }

        json_build_array_start(builder);
        bool has_put = false;

        for(length_t i = 0; i < call_info.length; i++){
            if(call_info.counts[i] > 0){
                if(has_put){
                    json_build_array_next(builder);
                }

                has_put = true;

                json_build_object_start(builder);
                json_build_object_key(builder, "name");
                json_build_string(builder, call_info.names[i]);
                json_build_object_next(builder);
                json_build_object_key(builder, "count");
                json_build_integer(builder, call_info.counts[i]);
                json_build_object_end(builder);
            }
        }

        json_build_array_end(builder);
    }

    json_build_array_end(builder);

    free(call_info.names);
    free(call_info.counts);
}

void handle_ast_query(query_t *query, json_builder_t *builder){
    bool lexing_succeeded = false;
    bool validation_succeeded = false;

    if(query->infrastructure == NULL){
        json_build_string(builder, "AST query is missing field 'infrastructure'");
        return;
    }

    if(query->filename == NULL){
        json_build_string(builder, "AST query is missing field 'filename'");
        return;
    }

    if(query->code == NULL){
        json_build_string(builder, "AST query is missing field 'code'");
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
    lexing_succeeded = true;

    strong_cstr_t identifierTokens = NULL;

    {
        json_builder_t identifierTokensBuilder;
        json_builder_init(&identifierTokensBuilder);

        build_identifierTokens(&identifierTokensBuilder, object);

        identifierTokens = json_builder_finalize(&identifierTokensBuilder);
    }
    
    if(parse(&compiler, object))        goto store_and_cleanup;
    validation_succeeded = true;

    length_t i;

store_and_cleanup:
    json_build_object_start(builder);
    json_build_object_key(builder, "validation");

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

    json_build_next(builder);
    json_build_object_key(builder, "ast");

    if(validation_succeeded){
        build_ast(builder, &compiler, object, query->features);
    } else {
        json_build_null(builder);
    }

    if(query->features & QUERY_FEATURE_INCLUDE_CALLS){
        json_build_next(builder);
        json_build_object_key(builder, "calls");

        build_calls(builder, &compiler, object, query->features);
    }

    json_build_array_next(builder);
    json_build_object_key(builder, "identifierTokens");
    
    if(lexing_succeeded){
        json_builder_append(builder, identifierTokens);
        free(identifierTokens);
    } else {
        json_build_null(builder);
    }
    json_build_object_end(builder);

cleanup:
    compiler_free(&compiler);
    return;
}
