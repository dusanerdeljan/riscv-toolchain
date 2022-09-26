#ifndef FUNC_PARAM_MAP_H
#define FUNC_PARAM_MAP_H

typedef struct func_entry {
    struct func_entry *next;        // next in the function chain
    struct param_entry *params;     // parameter chain
    int func_idx;                   // function index used as a key in the 'map'
} s_func_entry;

typedef struct param_entry {
    struct param_entry *next;       // next in the parameter chain
    unsigned param_type;            // type of the parameter
    unsigned param_idx;             // redundant field for easier search
} s_param_entry;

void init_func_param_map();

void declare_function(int func_idx);
void add_param(int func_idx, int param_idx, unsigned param_type);
unsigned get_param_type(int func_idx, int param_idx);

void clear_func_param_map();

#endif