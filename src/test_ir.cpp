#include "ir_struct.h"
#include "BinTrans.h"
#include "test_ir.h"

void test_ir() {
    /* test case:
     */


    BinTrans bt;
    bt.addFunction("a", 5);
//    bt.addFunction("b", 1);
//    ir_bblock blk;

//    bt.addBBlock("a", blk);
    bt.addBBlock("a");
    
    bt.addSymbol("t1", IN_FUNCTION, false, 0, false, 0, false);
    bt.addSymbol("t2", IN_FUNCTION, false, 0, false, 0, false);
    bt.addSymbol("t3", IN_FUNCTION, false, 0, false, 0, false);
    bt.addSymbol("res1", IN_FUNCTION, false, 0, false, 0, false);
    //bt.addSymbol("PRINTF", PROCEDURE_NAME, false, 0, false, 0, false);

    bt.addInstr("a", ARITHMETIC, IROP_ADD, ".param3", "t2", "res1");


    bt.addInstr("a", ARITHMETIC, IROP_ADD, "t1", "t2", "res1");
    bt.addInstr("a", ARITHMETIC, IROP_SUB, ".param4", "res1", "t3");
    bt.addInstr("a", CALL, IROP_CALL, "PRINTF", "trash", "trash2");
    bt.dumpSymbolTable();
    bt.emit();
    bt.dumpSymbolTable();
    bt.dumpRegContext();
}


void test_literial() {
    printf("====TEST_LITERIAL====\n");
    BinTrans bt;

    std::cout << "BindLiterial()" << std::endl;
    bt.bindLiterial(".func_const_255", 255);
    bt.bindLiterial(".func_const_253", 253);
    bt.dumpLiteral();
    std::cout << "Literial for .func_const_255: " << bt.getLiterial(".func_const_255") << std::endl;
    std::cout << "unbindLiterial()" << std::endl;
    bt.unbindLiterial(".func_const_255");
    std::cout << "isLiterial(\"abc\"): " << (bt.isLiterial("abc") ? "true" : "false") << std::endl;
    std::cout << "isLiterial(\".func_const_253\"): " <<  (bt.isLiterial(".func_const_253") ? "true" : "false") << std::endl;

    bt.dumpLiteral();
    printf("=====================\n");
}

void test_fib() {
    printf("===Testing for fib start===\n");
    printf(".ORIG x3000\n");
    /* Intermediate Level Desc.
    bb0: if (.param0 == 0) goto bb1 else bb2
    bb1: return 1
    bb2: 
        t1 = .param0 - 1
        if (t1 == 0) goto bb3 else bb4
    bb3: return 2
    bb4:
        t1 = n - 1
        t2 = n - 2
        t1 = fib(t1)
        t2 = fib(t2)
        t3 = t1 + t2
        return t3
    */
    BinTrans bt;
    bt.addFunction("fib", 1);
    bt.addSymbol("t1", IN_FUNCTION, false, -1, false, -1, false);
    bt.addSymbol("t2", IN_FUNCTION, false, -1, false, -1, false);
    bt.addSymbol("t3", IN_FUNCTION, false, -1, false, -1, false);

    bt.addBBlock("fib"); //bb0
    bt.addInstr("fib", BRANCH, IROP_IFZERO, ".param0", "bb1", "bb2");
    bt.addBBlock("fib"); //bb1

    //consts are emitted in epilogue.
    bt.addSymbol("const_2", L_CONST, false, -1, false, -1, false);
    bt.bindLiterial("const_2", 2);
    bt.addSymbol("const_1", L_CONST, false, -1, false, -1, false);
    bt.bindLiterial("const_1", 1);
    bt.addSymbol("const_0", L_CONST, false, -1, false, -1, false);
    bt.bindLiterial("const_0", 0);

    bt.addInstr("fib", RET, IROP_RETPARAM, "const_1", "", "");  //return 1
    bt.addBBlock("fib"); //bb2
    bt.addInstr("fib", ARITHMETIC, IROP_SUB, ".param0", "const_1", "t1");
    bt.addInstr("fib", BRANCH, IROP_IFZERO, "t1", "bb3", "bb4");
    bt.addBBlock("fib"); //bb3
    bt.addInstr("fib", RET, IROP_RETPARAM, "const_2", "", "");  //return 2
    bt.addBBlock("fib"); //bb4
    bt.addInstr("fib", ARITHMETIC, IROP_SUB, ".param0", "const_1", "t1");
    bt.addInstr("fib", ARITHMETIC, IROP_SUB, ".param0", "const_2", "t2");
    bt.addInstr("fib", CALL, IROP_PUSHARG, "t1", "", "");
    bt.addInstr("fib", CALL, IROP_CALLRET, "fib", "t1", ""); //return value sto in t1
    bt.addInstr("fib", CALL, IROP_PUSHARG, "t2", "", "");
    bt.addInstr("fib", CALL, IROP_CALLRET, "fib", "t2", ""); //return value sto in t2
    bt.addInstr("fib", ARITHMETIC, IROP_ADD, "t1", "t2", "t3");
    bt.addInstr("fib", RET, IROP_RETPARAM, "t3", "", "");  //return 2  

    bt.emit();
    printf(".END\n");
}