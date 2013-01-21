//
//  main.c
//  vmisc
//
//  Created by Michael Henderson on 1/18/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#include "VMisc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char myCode[16 * 1024];

int main(int argc, const char * argv[])
{
    // insert code here...
    printf("Hello, World!\n");

    memset(myCode, 0, 16 * 1024);
    char *testFile = "/Users/mdhender/Software/vmisc/vmisc/testFile.txt";
    FILE *fpTest = fopen(testFile, "r");
    if (!fpTest) {
        perror(testFile);
        return 2;
    }
    fread(myCode, sizeof(char), 16 * 1024 - 1, fpTest);
    fclose(fpTest);

    VMisc *vm = VMisc_New(4);
    if (!vm) {
        perror("new VM");
        return 2;
    }

    if (VMisc_Assemble(vm, myCode) != 0) {
        printf("\nerror:\tcould not assemble my code\n\n");
        return 2;
    }
    VMisc_Disassemble(vm, -1, -1);

    VMisc_Run(vm);
    printf(" info:\tvm exit value %8x\n", vm->exitValue);

    free(vm);

    return 0;
}

