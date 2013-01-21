//
//  VMisc.h
//  vmisc
//
//  Created by Michael Henderson on 1/19/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#ifndef vmisc_VMisc_h
#define vmisc_VMisc_h

#define VM_BITS_CODE  8
#define VM_BITS_VAL  24
#define VM_MASK_CODE (0xff000000)
#define VM_MASK_VAL  (~(VM_MASK_CODE))

#define VM_ASM(code,val)  ((code) << VM_BITS_VAL) ^ ((val) & VM_MASK_VAL)
#define VM_DISASM_CODE(x) (((x) & VM_MASK_CODE) >> VM_BITS_VAL)
#define VM_DISASM_VAL(x)  ((x) & VM_MASK_VAL)

#define INC_DS   idxDS = (idxDS + 1) % 128
#define INC_RS   idxRS = (idxRS + 1) % 128

#define POP_RS(vm)      vm->idxRS = vm->idxRS ? vm->idxRS-- : 127
#define PUSH_RS(vm, r) {vm->idxRS = (vm->idxRS + 1) % 128; vm->returnStack[vm->idxRS] = (r);}

#define OPC_NOOP      0x0000
#define OPC_CALL      0x0001
#define OPC_RETURN    0x0002
#define OPC_JUMP      0x0003
#define OPC_COROUTINE 0x0004
#define OPC_IFZERO    0x0005
#define OPC_IFNEG     0x0006
#define OPC_NEXT      0x0007
#define OPC_FETCH_A   0x0008
#define OPC_STORE_A   0x0009
#define OPC_FETCH_APP 0x000a
#define OPC_STORE_APP 0x000b
#define OPC_FETCH_B   0x000c
#define OPC_STORE_B   0x000d
#define OPC_FETCH_BPP 0x000e
#define OPC_STORE_BPP 0x000f
#define OPC_STORE_PC  0x0010
#define OPC_FETCH_PC  0x0011
#define OPC_PUSH      0x0012
#define OPC_POP       0x0013
#define OPC_DUP       0x0014
#define OPC_DROP      0x0015
#define OPC_OVER      0x0016
#define OPC_POP_A     0x0017
#define OPC_PUSH_A    0x0018
#define OPC_POP_B     0x0019
#define OPC_PUSH_B    0x0020
#define OPC_AND       0x0021
#define OPC_NOT       0x0022
#define OPC_XOR       0x0023
#define OPC_SHIFTL    0x0024
#define OPC_SHIFTR    0x0025
#define OPC_ADD       0x0026
#define OPC_MUL       0x0027
#define OPC_VALUE     0x00fd
#define OPC_STORAGE   0x00fe
#define OPC_HALT      0x00ff
#define OPC_LABEL     -1
#define OPC_INVALID   -2

// simulate core memory
//
typedef struct VMCore VMCore;
struct VMCore {
    int           coreSize;
    int           highWaterMark;
    unsigned int *core;
    unsigned int  data;
};

typedef struct VMisc VMisc;
struct VMisc {
    int coreSize;
    int highWaterMark;
    int exitValue;

    // registers
    //
    unsigned int pc;    // program counter
    unsigned int idxDS; // index into data stack
    unsigned int idxRS; // index into return stack
    unsigned int a;     // address/scratch
    unsigned int b;     // address

    // circular stacks
    //
    unsigned int dataStack[128];
    unsigned int returnStack[128];

    // core memory
    //
    unsigned int *core;
    unsigned int  data;
};

int    VMisc_Assemble(VMisc *vm, char *source);
int    VMisc_Disassemble(VMisc *vm, int startAt, int length);
VMisc *VMisc_New(int kb);
int    VMisc_Run(VMisc *vm);

#endif
