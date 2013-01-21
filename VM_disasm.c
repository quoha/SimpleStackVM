//
//  disasm.c
//  vmisc
//
//  Created by Michael Henderson on 1/19/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#include "VMisc.h"
#include <stdio.h>

int VMisc_Disassemble(VMisc *vm, int startAt, int length) {
    if (startAt < 0) {
        startAt = vm->pc;
    }
    if (length < 1) {
        length = vm->highWaterMark;
    }
    if (startAt == 0) {
        printf("..dis:\tcoreSize        %d\n", vm->coreSize);
        printf(".....:\thigh water mark %d\n", vm->highWaterMark);
    }

    printf(". dis:\tpc %04x r %8x d %3d/ %8x a %8x b %8x\n", vm->pc, vm->returnStack[vm->idxRS], vm->idxDS, vm->dataStack[vm->idxDS], vm->a, vm->b);

    if (startAt + length > vm->highWaterMark) {
        length = vm->highWaterMark - startAt;
    }

    int idx;
    for (idx = 0; idx < length ; idx++) {
        unsigned int code = VM_DISASM_CODE(vm->core[startAt + idx]);
        unsigned int val  = VM_DISASM_VAL(vm->core[startAt + idx]);
        const char *op = "??";
        switch (code) {
            case OPC_ADD      : op = "add"      ; break;
            case OPC_AND      : op = "and"      ; break;
            case OPC_CALL     : op = "call"     ; break;
            case OPC_COROUTINE: op = "coroutine"; break;
            case OPC_DROP     : op = "drop"     ; break;
            case OPC_DUP      : op = "dup"      ; break;
            case OPC_FETCH_A  : op = "fetch.a"  ; break;
            case OPC_FETCH_APP: op = "fetch.a++"; break;
            case OPC_FETCH_B  : op = "fetch.b"  ; break;
            case OPC_FETCH_BPP: op = "fetch.b++"; break;
            case OPC_FETCH_PC : op = "fetch.pc" ; break;
            case OPC_HALT     : op = "halt"     ; break;
            case OPC_IFNEG    : op = "ifneg"    ; break;
            case OPC_IFZERO   : op = "if"       ; break;
            case OPC_JUMP     : op = "jump"     ; break;
            case OPC_MUL      : op = "mul"      ; break;
            case OPC_NEXT     : op = "next"     ; break;
            case OPC_NOOP     : op = "noop"     ; break;
            case OPC_NOT      : op = "not"      ; break;
            case OPC_OVER     : op = "over"     ; break;
            case OPC_POP      : op = "pop"      ; break;
            case OPC_POP_A    : op = "pop.a"    ; break;
            case OPC_POP_B    : op = "pop.b"    ; break;
            case OPC_PUSH     : op = "push"     ; break;
            case OPC_PUSH_A   : op = "push.a"   ; break;
            case OPC_PUSH_B   : op = "push.b"   ; break;
            case OPC_RETURN   : op = "return"   ; break;
            case OPC_SHIFTL   : op = "shift.l"  ; break;
            case OPC_SHIFTR   : op = "shift.r"  ; break;
            case OPC_STORE_A  : op = "store.a"  ; break;
            case OPC_STORE_APP: op = "store.a++"; break;
            case OPC_STORE_B  : op = "store.b"  ; break;
            case OPC_STORE_BPP: op = "store.b++"; break;
            case OPC_STORE_PC : op = "store.pc" ; break;
            case OPC_XOR      : op = "xor"      ; break;
        }
        printf("%5x:\t%02x %06x    %-12s\n", startAt + idx, code, val, op);
    }
    
    return 0;
}
