//
//  asm.c
//  vmisc
//
//  Created by Michael Henderson on 1/19/13.
//  Copyright (c) 2013 Michael Henderson. All rights reserved.
//

#include "VMisc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// data structure modified from gperf
//
struct gperfHash {
    const char  *word;
    int          numberOfParms;
    unsigned int code;
} wordList[] = {
    {"add"      , 0, OPC_ADD      },
    {"and"      , 0, OPC_AND      },
    {"call"     , 1, OPC_CALL     },
    {"coroutine", 0, OPC_COROUTINE},
    {"drop"     , 0, OPC_DROP     },
    {"dup"      , 0, OPC_DUP      },
    {"fetch.a"  , 0, OPC_FETCH_A  },
    {"fetch.a++", 0, OPC_FETCH_APP},
    {"fetch.b"  , 0, OPC_FETCH_B  },
    {"fetch.b++", 0, OPC_FETCH_BPP},
    {"fetch.pc" , 0, OPC_FETCH_PC },
    {"halt"     , 1, OPC_HALT     },
    {"if"       , 1, OPC_IFZERO   },
    {"ifneg"    , 1, OPC_IFNEG    },
    {"jump"     , 1, OPC_JUMP     },
    {"mul"      , 0, OPC_MUL      },
    {"next"     , 1, OPC_NEXT     },
    {"noop"     , 0, OPC_NOOP     },
    {"not"      , 0, OPC_NOT      },
    {"over"     , 0, OPC_OVER     },
    {"pop"      , 0, OPC_POP      },
    {"pop.a"    , 0, OPC_POP_A    },
    {"pop.b"    , 0, OPC_POP_A    },
    {"push"     , 0, OPC_PUSH     },
    {"push.a"   , 0, OPC_PUSH_A   },
    {"push.b"   , 0, OPC_PUSH_B   },
    {"return"   , 0, OPC_RETURN   },
    {"shift.l"  , 0, OPC_SHIFTL   },
    {"shift.r"  , 0, OPC_SHIFTR   },
    {"storage"  , 1, OPC_STORAGE  },
    {"store.a"  , 0, OPC_STORE_A  },
    {"store.a++", 0, OPC_STORE_APP},
    {"store.b"  , 0, OPC_STORE_B  },
    {"store.b++", 0, OPC_STORE_BPP},
    {"store.pc" , 0, OPC_STORE_PC },
    {"v"        , 1, OPC_VALUE    },
    {"xor"      , 0, OPC_XOR      },
    {0          , 0, OPC_INVALID}
};

int HashValue(const char *word, int length) {
    if (length && word[length - 1] == ':') {
        return -1;
    } else {
        int idx;
        for (idx = 0; wordList[idx].word; idx++) {
            if (strcmp(word, wordList[idx].word) == 0) {
                if (wordList[idx].code == OPC_INVALID) {
                    return -2;
                }
                return idx;
            }
        }
    }
    return -2;
}

//
// isDefined  is set to 0 for labels that have not been defined.
//            when the label is defined, this is set to 1 and the
//            true offset is also set to the program counter (PC).
// lastUsedAt is used for back-patching. we allow forward references.
//            it's a single pass assembler, so when we find a forward
//            reference we will:
//               lookup the label in the symbol table
//               if not found
//                  create the entry
//                  set the defined flag to false
//                  set the last used at address to the PC
//               end-if
//               if entry is defined
//                  use the true offset
//               else
//                  use the last used at address for the op-value
//                  update the last used at address to the PC
//               end-if
// trueOffset contains the actual offset where the label is defined.
//
struct SymTabEntry {
    struct SymTabEntry *prev;
    const char         *label;
    int                 isDefined;
    int                 trueOffset;
    int                 lastUsedAt;
};

int VMisc_Assemble(VMisc *vm, char *src) {
    printf("  asm:\tcode is\n%s\n", src);
    struct SymTabEntry *tail = malloc(sizeof(*tail));
    if (!tail) {
        return 0;
    }
    tail->prev       = 0;
    tail->label      = "";
    tail->trueOffset = 0;
    tail->lastUsedAt = 0;
    tail->isDefined  = 1;

    // process source word by word
    //
    int line = 1;

    int pc = vm->highWaterMark;

    while (*src) {
        // skip whitespace, setting it to nil
        //
        while (*src && (*src == ';' || isspace(*src))) {
            if (*src == ';') {
                while (*src && *src != '\n') {
                    *(src++) = 0;
                }
            }
            if (*src == '\n') {
                line++;
            }
            *(src++) = 0;
        }
        if (!*src) {
            continue;
        }

        // pointer to word to assemble
        //
        int   wordLine  = line;
        char *word      = src;
        char *endOfWord = src;
        while (*src && !(*src == ';' || isspace(*src))) {
            endOfWord = src++;
        }

        // skip whitespace, setting it to nil
        //
        while (*src && (*src == ';' || isspace(*src))) {
            if (*src == ';') {
                while (*src && *src != '\n') {
                    *(src++) = 0;
                }
            }
            if (*src == '\n') {
                line++;
            }
            *(src++) = 0;
        }

        int lenWord   = (int)strlen(word);
        int hashKey   = HashValue(word, lenWord);

        if (hashKey == -2) {
            printf("  asm:\tinvalid word '%s' on line %d\n\n", word, wordLine);
            return 1;
        }

        // lookup the word (from gperf)
        //
        //if (lenWord <= MAX_WORD_LENGTH && lenWord >= MIN_WORD_LENGTH) {
        //    // from gperf hash() function
        //    //
        //    int hashValue = lenWord + asso_values[word[1]] + asso_values[word[lenWord - 1]];
        //    if (hashValue <= MAX_HASH_VALUE && hashValue >= 0) {
        //        struct gperfHash *s = wordList + hashValue;

        //       if (*word == s->word[0] && !strcmp (word + 1, s->word + 1)) {
        //            opCode = s->code;
        //        }
        //    }
        //}

        //printf("  asm:\t%s --> %02x\n", word, opCode);
        // will point to symbol table entry if needed
        //
        struct SymTabEntry *entry;

        if (hashKey == -1) {
            //printf("  asm:\tfound label '%s'\n", word);

            // we are defining the location of a label
            //
            // if label is in the symbol table
            //    back-patch the offset
            //    update with true offset
            // otherwise
            //    add the label to the symbol table
            //

            // check for invalid label
            //
            if (*endOfWord != ':') {
                printf("  asm:\tinvalid label '%s' on line %d\n\n", word, wordLine);
                return 1;
            }
            *endOfWord = 0;

            // find label in the symbol table
            //
            entry = tail;
            while (entry && strcmp(word, entry->label)) {
                entry = entry->prev;
            }
            //printf("  asm:\tlabel %san entry\n", entry ? "had " : "did not have ");

            if (!entry) {
                // add it to the symbol table
                //
                entry = malloc(sizeof(*entry));
                if (!entry) {
                    return 1;
                }
                entry->prev       = tail;
                entry->label      = word;
                entry->isDefined  = 1;
                entry->trueOffset = pc;
                entry->lastUsedAt = pc;
                tail              = entry;
                //printf("  asm:\tlabel %s added to symbol table with address %06x\n", entry->label, entry->lastUsedAt);
            } else {
                // update symbol table entry with label's true offset
                //
                entry->isDefined  = 1;
                entry->trueOffset = pc;

                // back-patch the code with the true offset. each address to be
                // patched contains the address of the previous use of the label.
                // follow these links to the first occurance. that will be the
                // only one to have the link be equal to the address.
                //
                //printf("  asm:\tpatching %s trueOffset is %06x\n", entry->label, entry->trueOffset);
                unsigned int prevAddress    = pc;
                unsigned int addressToPatch = entry->lastUsedAt;
                while (1) {
                    prevAddress = VM_DISASM_VAL(vm->core[addressToPatch]);
                    //printf("  asm:\tpatching %s addr %06x from %06x to %06x\n", entry->label, addressToPatch, prevAddress, pc);
                    vm->core[addressToPatch] = VM_ASM(VM_DISASM_CODE(vm->core[addressToPatch]), entry->trueOffset);
                    if (addressToPatch == prevAddress) {
                        break;
                    }
                    addressToPatch = prevAddress;
                }
            }

            // proceed to next word
            //
            continue;
        }

        int opCode = wordList[hashKey].code;
        
        // some op-codes don't require a second parameter
        //
        if (wordList[hashKey].numberOfParms == 0) {
            vm->core[pc]  = VM_ASM(opCode, 0);
            //printf("  asm:\t%5d %08x\n", pc, vm->core[pc]);
            pc++;
            continue;
        }

        char *parm = 0;
        int   parmIsNumber = 0;
        unsigned int parmValue = 0;

        // skip whitespace, setting it to nil
        //
        while (*src && (*src == ';' || isspace(*src))) {
            if (*src == ';') {
                while (*src && *src != '\n') {
                    *(src++) = 0;
                }
            }
            if (*src == '\n') {
                line++;
            }
            *(src++) = 0;
        }
        
        // pointer to word to assemble
        //
        parm = src;
        while (*src && !(*src == ';' || isspace(*src))) {
            src++;
        }
        
        // skip whitespace, setting it to nil
        //
        while (*src && (*src == ';' || isspace(*src))) {
            if (*src == ';') {
                while (*src && *src != '\n') {
                    *(src++) = 0;
                }
            }
            if (*src == '\n') {
                line++;
            }
            *(src++) = 0;
        }
        //printf("  asm:\tparm --> '%s'\n", parm);
        
        if (!*parm) {
            printf("\nerror:\tword '%s' on line %d requires a parameter\n\n", word, line);
            return 0;
        }
        
        if (HashValue(parm, (int)strlen(parm)) >= 0) {
            printf("  asm:\terror:\tinvalid paramemter '%s %s'\n\n", word, parm);
            return 1;
        }

        // determine if parameter is a label or value
        //
        char *p = parm;
        if (*p == '-' && isdigit(*(p+1))) {
            // allow a leading minus sign
            //
            p++;
        }
        // if the parm is all numbers, then it is not a label
        //
        while (*p && isdigit(*p)) {
            p++;
        }
        parmIsNumber = *p ? 0 : 1;
        if (parmIsNumber) {
            parmValue = atoi(parm);
            //printf("  asm:\tconverted parm %s to %d\n", parm, parmValue);
        } else {
            // find label in the symbol table
            //
            entry = tail;
            while (entry && strcmp(parm, entry->label)) {
                entry = entry->prev;
            }
            //printf("  asm:\tparm %s %s in symbol table\n", parm, entry ? "found" : "not found");
            
            // if it's not there, then add it
            //
            if (!entry) {
                entry = malloc(sizeof(*entry));
                if (!entry) {
                    return 1;
                }
                entry->prev       = tail;
                entry->label      = parm;
                entry->isDefined  = 0;
                entry->trueOffset = 0;
                entry->lastUsedAt = pc;
                tail              = entry;

                //printf("  asm:\tparm %s added to symbol table as %s\n", parm, entry->label);
            }
        }

        switch (opCode) {
            case OPC_STORAGE:
                if (!parmIsNumber) {
                    printf("  asm:\terror:\tstorage requires a number to follow\n\n");
                    return 1;
                }
                pc += parmValue;
                continue;
            case OPC_VALUE:
                if (parmIsNumber) {
                    vm->core[pc] = parmValue;
                } else if (entry->isDefined) {
                    vm->core[pc] = entry->trueOffset;
                } else {
                    vm->core[pc]      = entry->lastUsedAt;
                    entry->lastUsedAt = pc;
                }
                pc++;
                continue;
        }

        if (parmIsNumber) {
            vm->core[pc] = VM_ASM(opCode, parmValue);
        } else if (entry->isDefined) {
            vm->core[pc] = VM_ASM(opCode, entry->trueOffset);
        } else {
            vm->core[pc]      = VM_ASM(opCode, entry->lastUsedAt);
            entry->lastUsedAt = pc;
        }

        //printf("  asm:\t%5d %08x\n", pc, vm->core[pc]);
        pc++;
    }
    vm->highWaterMark = pc;

    int errorCount = 0;
    struct SymTabEntry *entry = tail;
    while (entry) {
        if (entry->isDefined) {
            printf("  asm:\tlabel %-16s is %04x\n", entry->label, entry->trueOffset);
        } else {
            printf("  asm:\terror:\tundefined label '%s'\n", entry->label);
            errorCount++;
        }
        entry = entry->prev;
    }

    return errorCount ? 1 : 0;
}

