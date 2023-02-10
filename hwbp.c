#include <stdlib.h>
#include "hwbp.h"
#include "dr.h"

PHWBP bp_create(LPVOID lpTarget, DWORD threadId, BP_READ_WRITE read_write, BP_LENGTH length)
{
    PHWBP bp = (PHWBP)malloc(sizeof(HWBP));
    if (!bp)
        return NULL;

    bp_init(bp, lpTarget, threadId, read_write, length);

    return bp;
}

void bp_destroy(PHWBP bp)
{
    free(bp);
}

static void bp_init(PHWBP bp, LPVOID lpTarget, DWORD threadId, BP_READ_WRITE read_write, BP_LENGTH length)
{
	bp->target = lpTarget;
	bp->threadId = threadId;
	bp->read_write = read_write;
	bp->length = length;
    bp->index = -1;
    bp->enabled = FALSE;
}

static int8_t get_free_index(dr7 _dr7)
{
	if (!_dr7.local_breakpoint_0)
		return 0;
    if (!_dr7.local_breakpoint_1)
        return 1;
    if (!_dr7.local_breakpoint_2)
        return 2;
    if (!_dr7.local_breakpoint_3)
        return 3;
	return -1; // if all are used, return
}

static BOOL bp_add_to_ctx(PHWBP bp, PCONTEXT ctx)
{
    dr7 _dr7;
    _dr7.flags = ctx->Dr7;

    int8_t idx = get_free_index(_dr7);

	if (idx == -1)
		return FALSE;

    switch (idx)
    {
    case 0:
        ctx->Dr0 = (DWORD64)bp->target;
        _dr7.local_breakpoint_0 = 1;
        _dr7.length_0 = bp->length;
        _dr7.read_write_0 = bp->read_write;
        break;
    case 1:
        ctx->Dr1 = (DWORD64)bp->target;
        _dr7.local_breakpoint_1 = 1;
        _dr7.length_1 = bp->length;
        _dr7.read_write_1 = bp->read_write;
        break;
    case 2:
        ctx->Dr2 = (DWORD64)bp->target;
        _dr7.local_breakpoint_2 = 1;
        _dr7.length_2 = bp->length;
        _dr7.read_write_2 = bp->read_write;
        break;
    case 3:
        ctx->Dr3 = (DWORD64)bp->target;
        _dr7.local_breakpoint_3 = 1;
        _dr7.length_3 = bp->length;
        _dr7.read_write_3 = bp->read_write;
        break;
    default:
		return FALSE; // should never happen
    }

	bp->index = idx;
    ctx->Dr7 = _dr7.flags;
	
	return TRUE;
}

static BOOL bp_remove_from_ctx(PHWBP bp, PCONTEXT ctx)
{
    dr7 _dr7;
    _dr7.flags = ctx->Dr7;

    switch (bp->index)
    {
    case 0:
        ctx->Dr0 = 0;
        _dr7.local_breakpoint_0 = 0;
        _dr7.length_0 = 0;
        _dr7.read_write_0 = 0;
        break;
    case 1:
        ctx->Dr1 = 0;
        _dr7.local_breakpoint_1 = 0;
        _dr7.length_1 = 0;
        _dr7.read_write_1 = 0;
        break;
    case 2:
        ctx->Dr2 = 0;
        _dr7.local_breakpoint_2 = 0;
        _dr7.length_2 = 0;
        _dr7.read_write_2 = 0;
        break;
    case 3:
        ctx->Dr3 = 0;
        _dr7.local_breakpoint_3 = 0;
        _dr7.length_3 = 0;
        _dr7.read_write_3 = 0;
        break;
	default:
		bp->enabled = FALSE;
        return FALSE; // ??
    }

	bp->index = -1;
    ctx->Dr7 = _dr7.flags;

	return TRUE;
}

BOOL bp_enable(PHWBP bp)
{
    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, bp->threadId);

    if (!hThread)
        return FALSE;

    if (SuspendThread(hThread) == (DWORD)-1)
    {
        CloseHandle(hThread);
        return FALSE;
    }

    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if (!GetThreadContext(hThread, &ctx))
    {
        ResumeThread(hThread);
        CloseHandle(hThread);
        return FALSE;
    }
    
    if (!bp_add_to_ctx(bp, &ctx))
	{
		ResumeThread(hThread);
		CloseHandle(hThread);
		return FALSE;
	}

    if (!SetThreadContext(hThread, &ctx))
    {
        ResumeThread(hThread);
        CloseHandle(hThread);
        return FALSE;
    }

    ResumeThread(hThread);
    CloseHandle(hThread);

    bp->enabled = TRUE;

    return TRUE;
}

BOOL bp_disable(PHWBP bp)
{
    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, FALSE, bp->threadId);

    if (!hThread)
        return FALSE;

    if (SuspendThread(hThread) == (DWORD)-1)
    {
        CloseHandle(hThread);
        return FALSE;
    }

    CONTEXT ctx = { 0 };
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    if (!GetThreadContext(hThread, &ctx))
    {
        ResumeThread(hThread);
        CloseHandle(hThread);
        return FALSE;
    }

    if (!bp_remove_from_ctx(bp, &ctx))
	{
		ResumeThread(hThread);
		CloseHandle(hThread);
		return FALSE;
	}

    if (!SetThreadContext(hThread, &ctx))
    {
        ResumeThread(hThread);
        CloseHandle(hThread);
        return FALSE;
    }

    ResumeThread(hThread);
    CloseHandle(hThread);

    bp->enabled = FALSE;

    return TRUE;
}
