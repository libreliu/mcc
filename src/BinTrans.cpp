#include "BinTrans.h"

// TODO (libreliu#1#): Debug on the program itself ...
//
//implement branch()


BinTrans::BinTrans() : f()
{
    //ctor
}

BinTrans::~BinTrans()
{
    //dtor
}

void BinTrans::addFunction(const char *func_name, int param_count) {
    ir_func func;
    if (strlen(func_name) >= MAX_FUNCNAME_LEN) {
        throw std::out_of_range("func_name too long");
        return;
    } else {
        for(std::list<ir_func>::iterator it = f.begin(); it != f.end(); ++it) {
            //std::cout << "Name:" << it->name << std::endl;
            if (!strcmp(it->name, func_name)) {
                throw std::out_of_range("duplicate function name");
                return;
            }
        }
        strcpy(func.name, func_name);
    }
    func.param_count = param_count;
    f.push_back(func);
}

void BinTrans::addBBlock(const char *func_name, const ir_bblock bblock) {
    ir_func func;
    for(std::list<ir_func>::iterator it = f.begin(); it != f.end(); ++it) {
        if (!strcmp(it->name, func_name)) {
                it->block.push_back(bblock);
                return;
        }
    }
    throw std::out_of_range("no matching function");
}

void BinTrans::addBBlock(const char *func_name) {
    ir_func func;
    ir_bblock blk_empty;
    for(std::list<ir_func>::iterator it = f.begin(); it != f.end(); ++it) {
        if (!strcmp(it->name, func_name)) {
                it->block.push_back(blk_empty);
                return;
        }
    }
    throw std::out_of_range("no matching function");
}

void BinTrans::emitConstTable() {
    for(std::list<ir_symbol>::iterator it = syms.begin(); it != syms.end(); it++) {
        if (it->type == L_CONST) {
            //it = syms.erase(it); //notice about *iterator validity*
            // //also remind to delete reg_context
            // for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
            //     if (ct.used[i] && (!strcmp(ct.use[i], name)) ) {
            //         ct.used[i] = false;
            //     }
            // }
            // return;
            printf("%s .FILL #%d\n", it->name, getLiterial(it->name));
        }
    }    
}

void BinTrans::addInstr(const char *func_name, ir_type type, ir_opcode opcode, const char *opa, const char *opb, const char *res) {
    char ops[3][MAX_SYMBOL_LEN + 1];
    strcpy(ops[0], opa);
    strcpy(ops[1], opb);
    strcpy(ops[2], res);
    char *opsd[3] = {ops[0], ops[1], ops[2]};
    this->addInstr(func_name, type, opcode, opsd);
}

void BinTrans::addInstr(const char *func_name, ir_type type, ir_opcode opcode, char *ops[]) {
    ir_inst inst;
    inst.opcode = opcode;
    inst.type = type;
    strcpy(inst.ops[0], ops[0]);
    strcpy(inst.ops[1], ops[1]);
    strcpy(inst.ops[2], ops[2]);


    //注意v1.end()指向的是最后一个元素的下一个位置，所以访问最后一个元素
    //的正确操作为：v1.end() - 1;

    for(std::list<ir_func>::iterator it = f.begin(); it != f.end(); ++it) {
        if (!strcmp(it->name, func_name)) {
            ir_bblock &itb = it->block.back();
            itb.inst.push_back(inst);
            return;
        }
    }
    throw std::out_of_range("no matching function");
}


void BinTrans::emitArithmetic(const ir_inst &inst) {
    ir_reg opa, opb, res, tmp;

    ir_reg prot[3];
    switch(inst.opcode) {
    case IROP_ADD:
        opa = getReg(inst.ops[0], prot, 0);
        prot[0] = opa;
        opb = getReg(inst.ops[1], prot, 1);
        prot[1] = opb;
        res = getReg(inst.ops[2], prot, 2);
        findSymbol(inst.ops[2])->dirty = true;
        printf("ADD R%d, R%d, R%d\n", res, opa, opb);

        break;
    case IROP_SUB:
        /* NOT tmp, opb
         * ADD res, opa, tmp
         */
        addSymbol(".arith_t", IN_FUNCTION, false, 0, false, 0, false);
        tmp = getReg(".arith_t", prot, 0);
        prot[0] = tmp;
        opb = getReg(inst.ops[1], prot, 1);
        printf("NOT R%d, R%d\n", tmp, opb);
        printf("ADD R%d, R%d, #1\n", tmp, tmp);
        //no need to protect opb
        opa = getReg(inst.ops[0], prot, 1);
        prot[1] = opa;
        res = getReg(inst.ops[2], prot, 2);
        findSymbol(inst.ops[2])->dirty = true;

        printf("ADD R%d, R%d, R%d\n", res, opa, tmp);
        delSymbol(".arith_t");
        break;
    }
}

void BinTrans::emit() {

    for(std::list<ir_func>::iterator it = f.begin(); it != f.end(); ++it) {
        PutLog("emitting function");
        this->emitFunction(*it);
    }
}

void BinTrans::dumpSymbolTable() {
    printf("Symbol Type inMem inReg MemAddr RegAddr Dirty\n");
    for(ir_symbol_table::iterator it = syms.begin(); it != syms.end(); ++it) {
        printf("%8s %d %5s %5s %8d %8d %5s\n"
               ,it->name
               ,it->type
               ,it->in_mem ? "true" : "false"
               ,it->in_reg ? "true" : "false"
               ,it->mem_addr
               ,it->reg_addr
               ,it->dirty ? "true" : "false");
    }
}

void BinTrans::emitBranch(const ir_inst &inst, const char *func_name) {

    ir_reg opa, opb, tmp, res;
    ir_reg prot[3];
    switch (inst.opcode) {
    case IROP_IFZERO:
        //emitBlockEpilogue : save everything back to memory

        //if ops[0] == 0 goto ops[1] else goto ops[2]
        this->emitWriteBack();
        opa = getReg(inst.ops[0], NULL, 0);
        this->discardRegRecord(inst.ops[0]);
        
        printf("ADD R%d, R%d, #0\n", opa, opa);
        printf("BRz %s_%s\n", func_name, inst.ops[1]);
        printf("BRnzp %s_%s\n", func_name, inst.ops[2]);
        break;

    }
}

void BinTrans::emitReturn(const ir_inst &inst, const char *func_name) {
    ir_reg opa;

    //write everything back
    this->emitWriteBack();
    switch (inst.opcode) {
    case IROP_RETPARAM:
        opa = getReg(inst.ops[0], NULL, 0);
        emitRegMove(0, opa);
        this->discardRegRecord(inst.ops[0]);

        printf("BRnzp %s_epilogue\n", func_name);
        break;
    case IROP_RETNPARAM:
        printf("BRnzp %s_epilogue\n", func_name);
        break;
    }
}

//used to discard its register allocation of a variable
void BinTrans::discardRegRecord(const char *sym_name) {
    ir_symbol *s;
    s = findSymbol(sym_name);
    s->in_reg = false;

    //clear ct
    for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
        if (ct.used[i] && (!strcmp(ct.use[i], sym_name)))
            ct.used[i] = false;
    }
}

void BinTrans::emitCall(const ir_inst &inst) {
    ir_symbol *s;
    ir_reg opa;
    ir_reg prot[1];

    switch (inst.opcode) {
    case IROP_PUSHARG:
        //just mark opa as args to be passed
        strcpy(ac.args[ac.arg_count], inst.ops[0]);
        ac.arg_count++;

        break;
    case IROP_CALL:
    case IROP_CALLRET:
        //save all reg. to mem (stack)
        /* Conditions for ct
         * 1. Not used => do nothing
         * 2.1 Used, but allocated in memory => if dirty, write; if not, discard
         * 2.2 Used, but not allocated in memory => write
         */
        for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
            if (ct.used[i] && ((s = findSymbol(ct.use[i]))->dirty == true) && (s->in_mem)) { // 2.1
                emitStackWrite(i, s->mem_addr);
                ct.used[i] = false;
                s->in_reg = false;
                s->dirty = false;
            } else if (ct.used[i] && (s->dirty == false) && (s->in_mem)) {
                ct.used[i] = false;
                s->in_reg = false;
            } else if (ct.used[i] && (s->in_mem == false)) {
                int mem_addr = emitPush(i);
                ct.used[i] = false;
                s->in_reg = false;
                s->in_mem = true;
                s->mem_addr = mem_addr;
            }

            //make sure that i've really done right
            assert(!ct.used[i]);
        }

        //prepare all params
        //*notice*: prepare param3~n first, for they could involve mem. reading
        //therefore use 0~2 as temp. storage



        for (int i = 3; i < ac.arg_count; i++) {
            //the rest lay in stack, starting from param 3
                if ((s = findSymbol(ac.args[i]))->in_mem) {
                    emitStackRead(0, s->mem_addr);
                    emitPush(0);
                } else if (s->in_reg) {
                    emitPush(s->reg_addr);
                } else {
                    throw std::out_of_range("unexpected situations in preparing params");
                    return;
                }
        }

        for (int i = 0; i < ac.arg_count && i < 3; i++) {
            //param 0, 1, 2 should be in register
            if ((s = findSymbol(ac.args[i]))->in_mem)
                emitStackRead(i, s->mem_addr);
            else if (s->in_reg && (s->reg_addr != i))
                emitRegMove(i, s->reg_addr);
            else {
                throw std::out_of_range("unexpected situations in preparing params");
                return;
            }
        }

        //toggle stack pointer to upper most
        if (sc.offset != 0)
            emitADD(6, 6, sc.offset);

        //time for making the jump
        emitJSR(inst.ops[0]);

        //restore stack
        if (sc.offset != 0)
            emitADD(6, 6, -sc.offset);

        sc.offset -= (ac.arg_count - 3 > 0 ? ac.arg_count - 3 : 0);

        //resetting context
        ac.arg_count = 0;
        if (inst.opcode == IROP_CALLRET) {
            //Store values in var
            prot[0] = 0; // retval passed in R0
            opa = this->getReg(inst.ops[1], prot, 1);
            emitADD(opa, 0, 0);
            findSymbol(inst.ops[1])->dirty = true;  //NECESSARY!
        }

        break;

    }
}

void BinTrans::explainInstr(const ir_inst &inst) {
    printf("; ");
    switch (inst.type) {
        case ARITHMETIC:
            printf("[ARITH] ");
            break;
        case BRANCH:
            printf("[BRANCH] ");
            break;
        case CALL:
            printf("[CALL] ");
            break;
        case RET:
            printf("[RET] ");
            break;
    }

    switch (inst.opcode) {
        case IROP_ADD:
            printf("(ADD, %s, %s, %s)", inst.ops[0], inst.ops[1], inst.ops[2]);
            break;
        case IROP_SUB:
            printf("(SUB, %s, %s, %s)", inst.ops[0], inst.ops[1], inst.ops[2]);
            break;
        case IROP_MUL:
            printf("(MUL, %s, %s, %s)", inst.ops[0], inst.ops[1], inst.ops[2]);
            break;
        case IROP_DIV:
            printf("(DIV, %s, %s, %s)", inst.ops[0], inst.ops[1], inst.ops[2]);
            break;
        case IROP_CALL:
            printf("(CALL, %s)", inst.ops[0]);
            break;
        case IROP_PUSHARG:
            printf("(PUSHARG, %s)", inst.ops[0]);
            break;
    }
    printf("\n");
}

void BinTrans::emitBlock(ir_bblock &block, const char *func_name) {
    bool skipWriteBack = false;

    for(std::list<ir_inst>::iterator it = block.inst.begin(); it != block.inst.end(); ++it) {
        //dumpRegContext();
        //dumpSymbolTable();
        explainInstr(*it);
        skipWriteBack = false;
        switch(it->type) {
        case ARITHMETIC:
            PutLog("emitting arithmetic");
            emitArithmetic(*it);
            break;
        case BRANCH:
            PutLog("emitting branch");
            emitBranch(*it, func_name);

            break;
        case CALL:
            PutLog("emitting call");
            emitCall(*it);

            break;
        case RET:
            PutLog("emitting ret");
            emitReturn(*it, func_name);
            skipWriteBack = true;
            break;
        }
    }

        //dumpRegContext();
        //dumpSymbolTable();
    //write everything back to memory (virt. register) before leaving the basic block
    if (!skipWriteBack)
        this->emitWriteBack();

}

//Write everything back to memory
void BinTrans::emitWriteBack() {
    ir_symbol *s;
    PutLog("writing everything back");
    for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
        if (ct.used[i] && ((s = findSymbol(ct.use[i]))->dirty == true) && (s->in_mem)) { // 2.1
            emitStackWrite(i, s->mem_addr);
            ct.used[i] = false;
            s->in_reg = false;
            s->dirty = false;
        } else if (ct.used[i] && (s->dirty == false) && (s->in_mem)) {
            ct.used[i] = false;
            s->in_reg = false;
        } else if (ct.used[i] && (s->in_mem == false)) {
            int mem_addr = emitPush(i);
            ct.used[i] = false;
            s->in_reg = false;
            s->in_mem = true;
            s->mem_addr = mem_addr;
        }
        //make sure that i've really done right
        assert(!ct.used[i]);
    }
}

void BinTrans::emitFunction(ir_func &func) {

    PutLog("emitting %s (%d params)", func.name, func.param_count);
    printf("; Function %s, with %d params\n%s\n", func.name, func.param_count, func.name);

    int i = 0;
    //Setting up register context
    if (func.param_count == 0) {
        for (int i = 0; i < TOTAL_REGISTER_COUNT; i++)
            ct.used[i] = false;
    } else if (func.param_count <= 3) {
        //parameters on registers
        for (int i = 0; i < func.param_count; i++) {
            ct.used[i] = true;

            char str[MAX_SYMBOL_LEN + 1];
            sprintf(str, ".param%d", i);
            strcpy(ct.use[i], str);
            addSymbol(str, PARAMETER, true, i, false, 0, false);
        }

    } else { //both on stack and mem
        for (int i = 0; i < 3; i++) {
            ct.used[i] = true;

            char str[MAX_SYMBOL_LEN + 1];
            sprintf(str, ".param%d", i);
            strcpy(ct.use[i], str);
            addSymbol(str, PARAMETER, true, i, false, 0, false);
        }

        for (int i = 3; i < func.param_count; i++) {
            char str[MAX_SYMBOL_LEN + 1];
            sprintf(str, ".param%d", i);
            //for parameter, mem_addr represents old_sp+(mem_addr)
            addSymbol(str, PARAMETER, false, -1, true, i + 1 - func.param_count, false);
        }

    }

    //context initialization
    sc.offset = 0;
    ac.arg_count = 0;

    //prologue
    printf("; %s_prologue\n", func.name);

    //saveR3, R4, R5, R7 - push in stack
    emitPush(3);
    emitPush(4);
    emitPush(5);
    emitPush(7);

    for(std::vector<ir_bblock>::iterator it = func.block.begin(); it != func.block.end(); ++it, i++) {
        PutLog("emitting %s :block %d", func.name, i);
        printf("%s_bb%d\n", func.name, i);
        emitBlock(*it, func.name);
    }

    //epilogue
    printf("%s_epilogue\n", func.name);

    //DO NOT USE emitPop, since sc.offset usually won't be zero.
    //Walkaround: manually emit here
    // emitPop(7);
    // emitPop(5);
    // emitPop(4);
    // emitPop(3);
    printf("LDR R7, R6, #4\nLDR R5, R6, #3\nLDR R4, R6, #2\nLDR R3, R6, #1\n");


    printf("RET\n");

    //consts
    printf("; %s.consts\n", func.name);
    emitConstTable();
}

void BinTrans::emitStackWrite(ir_reg reg, int offset) {
    printf("STR R%d, R6, #%d\n", reg, offset);
}

void BinTrans::emitStackRead(ir_reg reg, int offset) {
    printf("LDR R%d, R6, #%d\n", reg, offset);
}

int BinTrans::emitPush(ir_reg reg) {
    sc.offset++;
    printf("STR R%d, R6, #%d\n", reg, sc.offset);
    return sc.offset;
}

int BinTrans::emitPop(ir_reg reg) {
    printf("LDR R%d, R6, #%d\n", reg, sc.offset);
    sc.offset--;
    assert(sc.offset >= 0);
    return sc.offset;
}

void BinTrans::emitRegMove(ir_reg dest, ir_reg src) {
    printf("ADD R%d, R%d, #0\n", dest, src);
}

void BinTrans::emitJSR(const char *label) {
    printf("JSR %s\n", label);
}

void BinTrans::emitADD(ir_reg dest, ir_reg opa, int imm) {
    printf("ADD R%d, R%d, #%d\n", dest, opa, imm); //problematic?? TODO!!
}

void BinTrans::dumpRegContext() {
    printf("  Used  |  Use   \n");
    for(int i = 0; i < TOTAL_REGISTER_COUNT; i++) {
        printf("%8s %8s\n"
               ,ct.used[i] ? "true" : "false"
               ,ct.used[i] ? ct.use[i] : "N/A");
    }
}

void BinTrans::emitConstRead(ir_reg reg, const char *const_name) {
    //Only ints now
    printf("LD R%d, %s\n", reg, const_name);
}

ir_reg BinTrans::findAvailRegister(int *prot, int prot_len) {
        //Check if we have unused registers
        //**Avoid R7 and R6**
        for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
            if (!ct.used[i]) {
                bool isProtected = false;
                for (int j = 0; j < prot_len; j++) {
                    if (prot[j] == i) {
                        isProtected = true;
                        break;
                    }
                }
                if (isProtected)
                    continue;
                else return i;
            }
        }

        //We have no registers left - Spill one in
        //First, let's see if there is register that's not dirty, in memory and not in protect list
        ir_symbol *ss;
        for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
            if ( (ct.used[i] && (ss = findSymbol(ct.use[i]))->dirty == 0) && (ss->in_mem) && (ss->in_reg) ) {
                bool isProtected = false;
                for (int j = 0; j < prot_len; j++) {
                    if (prot[j] == i) {
                        isProtected = true;
                        break;
                    }
                }
                if (isProtected)
                    continue;
                else {
                    //Discard values in register, and update state
                    ss->in_reg = false;
                    return i;
                }
            }
        }

        //No such registers, pick one and write back
        //TODO: find out the least referenced one, but for easiness, i'll just pick the first one now
        //Notice that stack base didn't change during the whole phase

        for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
            if (ct.used[i]) {
                ss = findSymbol(ct.use[i]);
                if (ss->in_reg) {
                    bool isProtected = false;
                    for (int j = 0; j < prot_len; j++) {
                        if (prot[j] == i) {
                            isProtected = true;
                            break;
                        }
                    }
                    if (isProtected)
                        continue;

                    int mem_addr = emitPush(ss->reg_addr);
                    int ret_reg = ss->reg_addr;

                    ss->dirty = false;
                    ss->in_reg = false;
                    ss->mem_addr = mem_addr;
                    ss->in_mem = true;
                    return ret_reg;
                }
            }
        }
        throw std::out_of_range("No enough register left!\n");
        return -1;
}


//find register representation for sym_name
//allocate if needed
//int *v: violatile list
ir_reg BinTrans::getReg(const char *sym_name, int *prot, int prot_len) {
    PutLog("processing %s", sym_name);

    ir_symbol *s, *ss;
    int ret_reg;
    if ( (s = this->findSymbol(sym_name)) == NULL ) {
        throw std::out_of_range("invalid sym_name");
        return -1;
    }
    
    if (s->type == L_CONST) {
        if (s->in_reg)
            return s->reg_addr;

        //Actually, this depends on the current situation of PC.
        //If the literials are too far away, only LDR can handle this.
        //Here I'm using a risky approach - assuming the code no more than PCOff9
        ret_reg = findAvailRegister(prot, prot_len);
        this->emitConstRead(ret_reg, sym_name);

        ct.used[ret_reg] = true;
        strcpy(ct.use[ret_reg], sym_name);

        s->in_reg = true;
        s->reg_addr = ret_reg;
        s->dirty = false;
        return s->reg_addr;

    } else if (s->type == PARAMETER || s->type == IN_FUNCTION) {
        if (s->in_reg)
            return s->reg_addr;

        ret_reg = findAvailRegister(prot, prot_len);
        
        //In some time, the var is neither in mem or in reg, so we can avoid reading it in.
        //**This is supported, but not guaranteed to be ok, especially in interblock procedures.
        if (s->in_mem)
            this->emitStackRead(ret_reg, s->mem_addr);

        ct.used[ret_reg] = true;
        strcpy(ct.use[ret_reg], sym_name);

        s->in_reg = true;
        s->reg_addr = ret_reg;
        s->dirty = false;
        return s->reg_addr;
    } else {
        throw std::out_of_range("unsupported symbol type");
    }
}

ir_symbol *BinTrans::findSymbol(const char *sym_name) {
    //findSymbol
    for(std::list<ir_symbol>::iterator it = syms.begin(); it != syms.end(); ++it) {
        if (!strcmp(it->name, sym_name)) {
            return &(*it);
        }
    }
    return 0;
}

void BinTrans::addSymbol(const char *name,
                         ir_sym_type type,
                         bool in_reg,
                         ir_reg reg_addr,
                         bool in_mem,
                         ir_addr mem_addr,
                         bool dirty) {
    ir_symbol s;
    strcpy(s.name, name);
    s.type = type;
    s.in_mem = in_mem;
    s.in_reg = in_reg;
    s.reg_addr = reg_addr;
    s.mem_addr = mem_addr;
    s.dirty = dirty;

    PutLog("adding %s", s.name);
    syms.push_back(s);
}

void BinTrans::delSymbol(const char *name) {
    for(std::list<ir_symbol>::iterator it = syms.begin(); it != syms.end(); ) {
        if (!strcmp(it->name, name)) {
            it = syms.erase(it); //notice about *iterator validity*
            //also remind to delete reg_context
            for (int i = 0; i < TOTAL_REGISTER_COUNT - 2; i++) {
                if (ct.used[i] && (!strcmp(ct.use[i], name)) ) {
                    ct.used[i] = false;
                }
            }
            return;
        } else ++it;
    }
    throw std::out_of_range("error on delSymbol: not found");
}

void BinTrans::bindLiterial(const char *name, int literial) {
    //implicit conversion from const char * to std::string
    lc.local_int.insert(std::pair<std::string, int>(name, literial));
}

int BinTrans::getLiterial(const char *name) {
    std::map<std::string, int>::iterator it = lc.local_int.find(name);
    if (it == lc.local_int.end()) {
        throw std::out_of_range("getLiterial received an invalid name");
        return -1;
    } else {
        return it->second;
    }
}

void BinTrans::unbindLiterial(const char *name) {
    std::map<std::string, int>::iterator it = lc.local_int.find(name);
    if (it == lc.local_int.end()) {
        throw std::out_of_range("unbindLiterial received an invalid name");
        return;
    } else {
        lc.local_int.erase(it);
    }
}

void BinTrans::dumpLiteral() {
    printf("Symbol Value\n");
    for (auto i : lc.local_int) {
        printf("%6s %5d\n", (i.first).c_str(), i.second);
    }
}

bool BinTrans::isLiterial(const char *name) {
    std::map<std::string, int>::iterator it = lc.local_int.find(name);
    if (it == lc.local_int.end()) {
        return false;
    } else {
        return true;
    }
}
