//
//  run.c
//  vmisc
//
//  Created by Michael Henderson on 1/20/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#include "VMisc.h"
#include <stdio.h>

int VMisc_Run(VMisc *vm) {
    
    int maxCycles = 64;
    while (maxCycles-- > 0) {
        // instructions are 32 bits
        //    op-code  is  8 bits 31..24
        //    op-value is 24 bits 23..00

        // extract the op-code
        //
        unsigned int code = ((vm->core[vm->pc] & VM_MASK_CODE) >> VM_BITS_VAL);

        // extract the op-value
        //
        // TODO: sign extend val when needed
        unsigned int val  =  (vm->core[vm->pc] & VM_MASK_VAL);

        VMisc_Disassemble(vm, -1, 1);

        // increment the program counter
        //
        vm->pc++;

        unsigned int tmp;
        unsigned int x1, x2;

        switch (code) {
            case OPC_ADD:
                // DS x1 x2 - (x1+x2)
                //
                
                // pop x2
                //
                x2 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // pop x1
                //
                x1 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // push (x1 + x2)
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = (x1 + x2);
                break;
            case OPC_AND:
                // DS x1 x2 - (x1&x2)
                //
                
                // pop x2
                //
                x2 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // pop x1
                //
                x1 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // push (x1 & x2)
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = (x1 & x2);
                break;
            case OPC_CALL:
                // push PC onto return stack
                //
                vm->idxRS++;
                vm->idxRS %= 128;
                vm->returnStack[vm->idxRS] = vm->pc;
                
                // set PC to op-value
                //
                vm->pc = val;
                
                break;
            case OPC_COROUTINE:
                // (PC, RS) = (RS, PC)
                //
                tmp = vm->returnStack[vm->idxRS];
                vm->returnStack[vm->idxRS] = vm->pc;
                vm->pc = tmp;
                break;
            case OPC_DROP:
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                break;
            case OPC_DUP:
                // duplicate DS onto DS
                //
                tmp = vm->dataStack[vm->idxDS];
                
                // push tmp onto DS
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = tmp;
                break;
            case OPC_FETCH_A:
            case OPC_FETCH_APP:
                // push [A] onto DS
                //
                if (vm->a > vm->coreSize) {
                    printf("   vm:\terror:\tfetch.a out of bounds %04x / %04x\n\n", vm->a, vm->coreSize);
                    return 1;
                }
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->core[vm->a];
                
                if (code == OPC_FETCH_APP) {
                    vm->a++;
                }
                break;
            case OPC_FETCH_B:
            case OPC_FETCH_BPP:
                // push [B] onto DS
                //
                if (vm->b > vm->coreSize) {
                    printf("   vm:\terror:\tfetch.b out of bounds %04x / %04x\n\n", vm->b, vm->coreSize);
                    return 1;
                }
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->core[vm->b];

                if (code == OPC_FETCH_BPP) {
                    vm->b++;
                }
                break;
            case OPC_FETCH_PC:
                // push [PC] onto DS
                //
                if (vm->pc > vm->coreSize) {
                    printf("   vm:\terror:\tfetch.pc out of bounds %04x / %04x\n\n", vm->pc, vm->coreSize);
                    return 1;
                }
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->core[vm->pc];
                vm->pc++;
                break;
            case OPC_JUMP:
                // unconditional jump
                //
                vm->pc = val;
                break;
            case OPC_HALT:
                // end execution
                //
                vm->exitValue = val;
                return 0;
            case OPC_IFNEG:
                // if DS[0] is < zero PC = val
                //
                if ((int)(vm->dataStack[vm->idxDS]) < 0) {
                    vm->pc = val;
                }
                break;
            case OPC_IFZERO:
                // if DS[0] is zero PC = val
                //
                if (vm->dataStack[vm->idxDS] == 0) {
                    vm->pc = val;
                }
                break;
            case OPC_MUL:
                // DS x1 x2 - (x1*x2)
                //
                
                // pop x2
                //
                x2 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // pop x1
                //
                x1 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // push (x1 + x2)
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = (x1 * x2);
                break;
            case OPC_NEXT:
                // if RS != 0
                //   PC = val
                //   RS--
                // else
                //   pop RS
                //
                if (vm->returnStack[vm->idxRS] == 0) {
                    // pop RS
                    //
                    if (vm->idxRS == 0) {
                        vm->idxRS = 127;
                    } else {
                        vm->idxRS--;
                    }
                } else {
                    vm->returnStack[vm->idxRS]--;
                    vm->pc = val;
                }
            case OPC_NOOP:
                // do nothing
                //
                break;
            case OPC_NOT:
                // DS x1 - (~x1)
                //
                vm->dataStack[vm->idxDS] = ~(vm->dataStack[vm->idxDS]);
                break;
            case OPC_OVER:
                // DS a b - a b a
                //
                tmp = (vm->idxDS == 0) ? vm->dataStack[127] : vm->dataStack[vm->idxDS - 1];
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->dataStack[tmp];
                break;
            case OPC_POP:
                // push RS to DS
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->returnStack[vm->idxRS];
                
                // pop RS
                //
                if (vm->idxRS == 0) {
                    vm->idxRS = 127;
                } else {
                    vm->idxRS--;
                }
                break;
            case OPC_POP_A:
                // copy DS to A
                //
                vm->a = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                break;
            case OPC_POP_B:
                // copy DS to B
                //
                vm->b = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                break;
            case OPC_PUSH:
                // push DS to RS
                //
                vm->idxRS = (vm->idxRS + 1) % 128;
                vm->returnStack[vm->idxRS] = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                break;
            case OPC_PUSH_A:
                // push A to DS
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->a;
                break;
            case OPC_PUSH_B:
                // push B to DS
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = vm->b;
                break;
            case OPC_RETURN:
                // PC = RS
                //
                vm->pc = vm->returnStack[vm->idxRS];

                // pop RS
                //
                if (vm->idxRS == 0) {
                    vm->idxRS = 127;
                } else {
                    vm->idxRS--;
                }
                break;
            case OPC_SHIFTL:
                // DS x1 - (x1<<1)
                //
                vm->dataStack[vm->idxDS] = (vm->dataStack[vm->idxDS] << 1);
                break;
            case OPC_SHIFTR:
                // DS x1 - (x1>>1)
                //
                vm->dataStack[vm->idxDS] = (vm->dataStack[vm->idxDS] >> 1);
                break;
            case OPC_STORE_A:
            case OPC_STORE_APP:
                // [A] = DS
                //
                if (vm->a > vm->coreSize) {
                    printf("   vm:\terror:\tstore.a out of bounds %04x / %04x\n\n", vm->a, vm->coreSize);
                    return 1;
                }
                vm->core[vm->a] = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                if (code == OPC_STORE_APP) {
                    vm->a++;
                }
                break;
            case OPC_STORE_B:
            case OPC_STORE_BPP:
                // [B] = DS
                //
                if (vm->b > vm->coreSize) {
                    printf("   vm:\terror:\tstore.b out of bounds %04x / %04x\n\n", vm->b, vm->coreSize);
                    return 1;
                }
                vm->core[vm->b] = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                if (code == OPC_STORE_BPP) {
                    vm->b++;
                }
                break;
            case OPC_STORE_PC:
                // [PC] = DS
                //
                if (vm->pc - 1 > vm->coreSize) {
                    printf("   vm:\terror:\tstore.pc out of bounds %04x / %04x\n\n", vm->pc, vm->coreSize);
                    return 1;
                }
                vm->core[vm->pc - 1] = vm->dataStack[vm->idxDS];
                
                // pop DS
                //
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                break;
            case OPC_XOR:
                // DS x1 x2 - (x1^x2)
                //
                
                // pop x2
                //
                x2 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // pop x1
                //
                x1 = vm->dataStack[vm->idxDS];
                if (vm->idxDS == 0) {
                    vm->idxDS = 127;
                } else {
                    vm->idxDS--;
                }
                
                // push (x1^x2)
                //
                vm->idxDS = (vm->idxDS + 1) % 128;
                vm->dataStack[vm->idxDS] = (x1 ^ x2);
                break;
            default:
                printf(" halt:\tunrecognized op-code %02x\n", code);
                return 1;
        }
        vm->pc %= vm->coreSize;
    }
    
    return 0;
}
