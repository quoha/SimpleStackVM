//
//  new.c
//  vmisc
//
//  Created by Michael Henderson on 1/20/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#include "VMisc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VMisc *VMisc_New(int kb) {
    if (kb < 1) {
        kb = 1;
    } else if (kb > 64) {
        kb = 64;
    }
    kb = 1;
    
    int coreSize = kb * 1024;
    
    VMisc *vm = malloc(sizeof(*vm) + (sizeof(int) * coreSize));
    if (vm) {
        memset(vm, 0, sizeof(*vm) + (sizeof(int) * coreSize));
        vm->coreSize      = coreSize;
        vm->highWaterMark = 0;
        vm->core          = &(vm->data);
        printf("hey %p %p\n", vm, vm->core);
    }
    
    return vm;
}

