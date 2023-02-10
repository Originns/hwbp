#pragma once

#include <Windows.h>
#include "dr.h"

typedef enum
{
    INSTRUCTION_EXECUTION = 0,
    DATA_WRITEONLY = 1,
    IO_READWRITE = 2,
    DATA_READWRITE = 3
} BP_READ_WRITE;

typedef enum
{
    ONE_BYTE = 0,
    TWO_BYTE = 1,
    EIGHT_BYTE = 2,
    FOUR_BYTE = 3
} BP_LENGTH;

typedef struct _HWBP
{
    LPVOID target;
    DWORD threadId;
    BP_READ_WRITE read_write;
    BP_LENGTH length;
    int8_t index;
    uint8_t enabled;
} HWBP, *PHWBP;

EXTERN_C_START

PHWBP bp_create(LPVOID lpTarget, DWORD threadId, BP_READ_WRITE read_write, BP_LENGTH length);
BOOL bp_enable(PHWBP bp);
BOOL bp_disable(PHWBP bp);
void bp_destroy(PHWBP bp);

EXTERN_C_END
