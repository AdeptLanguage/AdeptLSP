
#include "AST/TYPE/ast_type_set.h"
#include "AST/ast_type.h"
#include "AST/TYPE/ast_type_hash.h"
#include "AST/TYPE/ast_type_identical.h"

static ast_type_t *ast_type_clone_on_heap(const ast_type_t *type){
    ast_type_t *result = malloc(sizeof(ast_type_t));
    *result = ast_type_clone(type);
    return result;
}

static hash_t ast_type_set_hash_function(const void *value) {
    return ast_type_hash((const ast_type_t*) value);
}

static bool ast_type_set_equals_function(const void *a, const void *b){
    return ast_types_identical((const ast_type_t*) a, (const ast_type_t*) b);
}

static void *ast_type_set_preinsert_clone_function(const void *value){
    return ast_type_clone_on_heap((const ast_type_t*) value);
}

static void ast_type_set_free_function(void *value){
    ast_type_free_fully((ast_type_t*) value);
}

void ast_type_set_init(ast_type_set_t *set, length_t num_buckets){
    set_init(&set->impl, num_buckets, &ast_type_set_hash_function, &ast_type_set_equals_function, &ast_type_set_preinsert_clone_function);
}

bool ast_type_set_insert(ast_type_set_t *set, ast_type_t *type){
    return set_insert(&set->impl, (void*) type);
}

void ast_type_set_traverse(ast_type_set_t *set, void (*run_func)(ast_type_t*)){
    set_traverse(&set->impl, (set_traverse_func_t) run_func);
}

void ast_type_set_free(ast_type_set_t *set){
    set_free(&set->impl, &ast_type_set_free_function);
}
