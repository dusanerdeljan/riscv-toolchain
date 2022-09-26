#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "func_param_map.h"

s_func_entry *param_map;

// ===== 'PRIVATE' functions =====

s_func_entry *get_func_entry(int func_idx) {
    s_func_entry *current = param_map;
    while (current != NULL) {
        if (current->func_idx == func_idx) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void clear_params(s_func_entry *func) {
    if (func == NULL) {
        return;
    }
    while (func->params != NULL) {
        s_param_entry *current = func->params;
        func->params = current->next;
        free(current);
    }
    free(func);
}

// ==== 'PUBLIC' functions ====

void init_func_param_map() {
    param_map = NULL;
}

// Always adds to the beggining of the list beacuse it is easier :D
void declare_function(int func_idx) {
    s_func_entry *func = (s_func_entry*) malloc(sizeof(s_func_entry));
    func->next = param_map;
    func->params = NULL;
    func->func_idx = func_idx;
    param_map = func;
}

void add_param(int func_idx, int param_idx, unsigned param_type) {
    s_param_entry *param = (s_param_entry*) malloc(sizeof(s_param_entry));
    param->param_idx = param_idx;
    param->param_type = param_type;
    s_func_entry *func = get_func_entry(func_idx);
    param->next = func->params;
    func->params = param;
}

unsigned get_param_type(int func_idx, int param_idx) {
    s_func_entry *func = get_func_entry(func_idx);
    s_param_entry* param = func->params;
    while (param != NULL) {
        if (param->param_idx == param_idx) {
            return param->param_type;
        }
        param = param->next;
    }
    return NO_TYPE;
}
    

void clear_func_param_map() {
    while (param_map != NULL) {
        s_func_entry *current = param_map;
        param_map = param_map->next;
        clear_params(current);
    }
}