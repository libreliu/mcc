#ifndef BINTRANS_H
#define BINTRANS_H

#include <assert.h>

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "ir_struct.h"

#ifdef DEBUG
#define PutLog(...) do{                                           \
                           printf("[%s, %d] ", __func__, __LINE__);      \
                           printf(__VA_ARGS__);                      \
                           printf("\n"); \
                          } while (0)
#else
#define PutLog(...) do{ } while (0)
#endif

class BinTrans {
    public:
        BinTrans();
        ~BinTrans();

        void addFunction(const char *func_name, int param_count);
        void addBBlock(const char *func_name, const ir_bblock bblock);
        void addBBlock(const char *func_name);
        void emit();

        void addInstr(const char *func_name, ir_type type, ir_opcode opcode, const char *opa, const char *opb, const char *res);
        void addInstr(const char *func_name, ir_type type, ir_opcode opcode, char *ops[]);

        void dumpRegContext();
        void dumpSymbolTable();
        void addSymbol(const char *name,
                        ir_sym_type type,
                        bool in_reg,
                        ir_reg reg_addr,
                        bool in_mem,
                        ir_addr mem_addr,
                        bool dirty);
        void delSymbol(const char *name);

        //used for symbol whose type is Local Constant
        void bindLiterial(const char *name, int literial);
        void unbindLiterial(const char *name);
        int getLiterial(const char *name);
        void dumpLiteral();
        bool isLiterial(const char *name);
    protected:

        ir_reg_context ct;
        ir_symbol_table syms;
        ir_stack_context sc;
        ir_literal_context lc;

        //Argument passing translations
        ir_arg_context ac;

        //context-related params
        

        std::list<ir_func> f;
        void emitFunction(ir_func &func);
        void emitBlock(ir_bblock &block, const char *func_name);
        void emitArithmetic(const ir_inst &inst);
        void emitCall(const ir_inst &inst);
        void emitBranch(const ir_inst &inst, const char *func_name);
        void emitReturn(const ir_inst &inst, const char *func_name);

        void discardRegRecord(const char *sym_name);
        void emitWriteBack();
        ir_reg getReg(const char *sym_name, int *prot, int prot_len);

        void emitConstRead(ir_reg reg, const char *const_name);

        void emitConstTable();

        ir_symbol *findSymbol(const char *sym_name);
        ir_reg findAvailRegister(int *prot, int prot_len);

        //push ir_reg to stack
        int emitPush(ir_reg reg);
        int emitPop(ir_reg reg);
        void emitStackRead(ir_reg reg, int offset);
        void emitStackWrite(ir_reg reg, int offset);
        void emitRegMove(ir_reg dest, ir_reg src);
        void emitJSR(const char *label);
        void emitADD(ir_reg dest, ir_reg opa, int imm);

        void explainInstr(const ir_inst &inst);
};

#endif // BINTRANS_H
