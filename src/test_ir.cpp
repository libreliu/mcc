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
    printf("===Testing for fib start===");
    /* Intermediate Level Desc.
    bb0: if (.param0 == 0) goto bb1 else bb2
    bb1: return 1
    bb2: if (.param0 == 1) goto bb3 else bb4
    bb3: return 2
    bb4: t1 = fib(n-1)
        t2 = fib(n-2)
        t3 = t1 + t2
        return t3
    */
    BinTrans bt;
    bt.addFunction("fib", 1);
    bt.addBBlock("fib");
    bt.addInstr("fib", BRANCH, IROP_IFZERO, ".param0", "bb1", "bb2");
    bt.addBBlock("fib");
    //bt.addInstr("fib", RET, IROP_RETPARAM, );  //return 1

    bt.addSymbol("t1", IN_FUNCTION, false, -1, false, -1, false);
    bt.addSymbol("t2", IN_FUNCTION, false, -1, false, -1, false);
    bt.addSymbol("t3", IN_FUNCTION, false, -1, false, -1, false);
}