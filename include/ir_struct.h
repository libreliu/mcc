#ifndef IR_STRUCT_H_INCLUDED
#define IR_STRUCT_H_INCLUDED

#include <vector>
#include <list>
#include <map>

#define IROP_ADD 1
#define IROP_SUB 2
#define IROP_MUL 3
#define IROP_DIV 4


//ops[0] = arg
#define IROP_PUSHARG 10
//ops[0] = target
#define IROP_CALL 20

//return without param
#define IROP_RETNPARAM 21
//return with param
#define IROP_RETPARAM 22

//if opa == 0 goto res
#define IROP_IFZERO 30
#define IROP_IFEQ 31

#define MAX_ARGS 20
#define MAX_SYMBOL_LEN 30
#define MAX_FUNCNAME_LEN 30
#define TOTAL_REGISTER_COUNT 8

//IN_FUNCTION: Stack temporaries
//PARAMETER: Program arguments received
//ARGUMENT: Program arguments to be passed
//PROCEDURE_NAME: Symbols of other functions to be called
//L_CONST: Local Constant

typedef enum {IN_FUNCTION, PARAMETER, L_CONST, ARGUMENT, PROCEDURE_NAME} ir_sym_type;
typedef short ir_addr;
typedef int ir_reg;

typedef struct ir_symbol {
    char name[MAX_SYMBOL_LEN + 1];
    ir_sym_type type;
    bool in_mem;
    ir_addr mem_addr;
    bool in_reg;
    ir_reg reg_addr;
    /* Dirty = inconsistent between mem and reg, need write back */
    bool dirty;
} ir_symbol;

typedef std::list<ir_symbol> ir_symbol_table;

typedef int ir_opcode;
typedef enum {ARITHMETIC, BRANCH, CALL, RET} ir_type;

typedef struct ir_inst {
    ir_type type;
    ir_opcode opcode;
    /* ops[0] == opa;
     * ops[1] == opb;
     * ops[2] == dest
     */
    char ops[3][MAX_SYMBOL_LEN + 1];
} ir_inst;

typedef struct ir_reg_context {
    bool used[TOTAL_REGISTER_COUNT];
    //pointed to ir_symbol entry via name
    char use[TOTAL_REGISTER_COUNT][MAX_SYMBOL_LEN + 1];
} ir_reg_context;

typedef struct ir_stack_context {
    int offset;
} ir_stack_context;

typedef struct ir_bblock {
    std::list<ir_inst> inst;
} ir_bblock;

typedef struct ir_func {
    char name[MAX_FUNCNAME_LEN + 1];
    int param_count;
    std::vector<ir_bblock> block;
} ir_func;

typedef struct ir_arg_context {
    int arg_count;
    char args[MAX_ARGS][MAX_SYMBOL_LEN + 1];
} ir_arg_context;

typedef struct ir_literal_context {
    std::map<std::string, int> local_int;
} ir_literal_context;

#endif // IR_STRUCT_H_INCLUDED
