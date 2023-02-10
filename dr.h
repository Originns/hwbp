#pragma once
#include <stdint.h>

/**
 * @defgroup debug_registers \
 *           Debug registers
 *
 * Eight debug registers control the debug operation of the processor. These registers can be written to and read using the
 * move to/from debug register form of the MOV instruction. A debug register may be the source or destination operand for
 * one of these instructions.
 * Debug registers are privileged resources; a MOV instruction that accesses these registers can only be executed in
 * real-address mode, in SMM or in protected mode at a CPL of 0. An attempt to read or write the debug registers from any
 * other privilege level generates a general-protection exception (\#GP). The primary function of the debug registers is to
 * set up and monitor from 1 to 4 breakpoints, numbered 0 though 3. For each breakpoint, the following information can be
 * specified:
 * - The linear address where the breakpoint is to occur.
 * - The length of the breakpoint location: 1, 2, 4, or 8 bytes.
 * - The operation that must be performed at the address for a debug exception to be generated.
 * - Whether the breakpoint is enabled.
 * - Whether the breakpoint condition was present when the debug exception was generated.
 *
 * @see Vol3B[17.2.4(Debug Control Register (DR7))]
 * @see Vol3B[17.2(DEBUG REGISTERS)] (reference)
 * @{
 */
typedef union
{
    struct
    {
        /**
         * @brief B0 through B3 (breakpoint condition detected) flags
         *
         * [Bits 3:0] Indicates (when set) that its associated breakpoint condition was met when a debug exception was generated.
         * These flags are set if the condition described for each breakpoint by the LENn, and R/Wn flags in debug control register
         * DR7 is true. They may or may not be set if the breakpoint is not enabled by the Ln or the Gn flags in register DR7.
         * Therefore on a \#DB, a debug handler should check only those B0-B3 bits which correspond to an enabled breakpoint.
         */
        uint64_t breakpoint_condition : 4;
        uint64_t reserved1 : 9;

        /**
         * @brief BD (debug register access detected) flag
         *
         * [Bit 13] Indicates that the next instruction in the instruction stream accesses one of the debug registers (DR0 through
         * DR7). This flag is enabled when the GD (general detect) flag in debug control register DR7 is set.
         *
         * @see Vol3B[17.2.4(Debug Control Register (DR7))]
         */
        uint64_t debug_register_access_detected : 1;

        /**
         * @brief BS (single step) flag
         *
         * [Bit 14] Indicates (when set) that the debug exception was triggered by the singlestep execution mode (enabled with the
         * TF flag in the EFLAGS register). The single-step mode is the highestpriority debug exception. When the BS flag is set,
         * any of the other debug status bits also may be set.
         */
        uint64_t single_instruction : 1;

        /**
         * @brief BT (task switch) flag
         *
         * [Bit 15] Indicates (when set) that the debug exception was triggered by the singlestep execution mode (enabled with the
         * TF flag in the EFLAGS register). The single-step mode is the highestpriority debug exception. When the BS flag is set,
         * any of the other debug status bits also may be set.
         */
        uint64_t task_switch : 1;

        /**
         * @brief RTM (restricted transactional memory) flag
         *
         * [Bit 16] Indicates (when clear) that a debug exception (\#DB) or breakpoint exception (\#BP) occurred inside an RTM
         * region while advanced debugging of RTM transactional regions was enabled. This bit is set for any other debug exception
         * (including all those that occur when advanced debugging of RTM transactional regions is not enabled). This bit is always
         * 1 if the processor does not support RTM.
         *
         * @see Vol3B[17.3.3(Debug Exceptions, Breakpoint Exceptions, and Restricted Transactional Memory (RTM))]
         */
        uint64_t restricted_transactional_memory : 1;
        uint64_t reserved2 : 47;
    };

    uint64_t flags;
} dr6;

typedef union
{
    struct
    {
        /**
         * @brief L0 through L3 (local breakpoint enable) flags (bits 0, 2, 4, and 6)
         *
         * [Bit 0] Enables (when set) the breakpoint condition for the associated breakpoint for the current task. When a
         * breakpoint condition is detected and its associated Ln flag is set, a debug exception is generated. The processor
         * automatically clears these flags on every task switch to avoid unwanted breakpoint conditions in the new task.
         */
        uint64_t local_breakpoint_0 : 1;

        /**
         * @brief G0 through G3 (global breakpoint enable) flags (bits 1, 3, 5, and 7)
         *
         * [Bit 1] Enables (when set) the breakpoint condition for the associated breakpoint for all tasks. When a breakpoint
         * condition is detected and its associated Gn flag is set, a debug exception is generated. The processor does not clear
         * these flags on a task switch, allowing a breakpoint to be enabled for all tasks.
         */
        uint64_t global_breakpoint_0 : 1;
        uint64_t local_breakpoint_1 : 1;
        uint64_t global_breakpoint_1 : 1;
        uint64_t local_breakpoint_2 : 1;
        uint64_t global_breakpoint_2 : 1;
        uint64_t local_breakpoint_3 : 1;
        uint64_t global_breakpoint_3 : 1;

        /**
         * @brief LE (local exact breakpoint enable)
         *
         * [Bit 8] This feature is not supported in the P6 family processors, later IA-32 processors, and Intel 64 processors. When
         * set, these flags cause the processor to detect the exact instruction that caused a data breakpoint condition. For
         * backward and forward compatibility with other Intel processors, we recommend that the LE and GE flags be set to 1 if
         * exact breakpoints are required.
         */
        uint64_t local_exact_breakpoint : 1;
        uint64_t global_exact_breakpoint : 1;
        uint64_t reserved1 : 1;

        /**
         * @brief RTM (restricted transactional memory) flag
         *
         * [Bit 11] Enables (when set) advanced debugging of RTM transactional regions. This advanced debugging is enabled only if
         * IA32_DEBUGCTL.RTM is also set.
         *
         * @see Vol3B[17.3.3(Debug Exceptions, Breakpoint Exceptions, and Restricted Transactional Memory (RTM))]
         */
        uint64_t restricted_transactional_memory : 1;
        uint64_t reserved2 : 1;

        /**
         * @brief GD (general detect enable) flag
         *
         * [Bit 13] Enables (when set) debug-register protection, which causes a debug exception to be generated prior to any MOV
         * instruction that accesses a debug register. When such a condition is detected, the BD flag in debug status register DR6
         * is set prior to generating the exception. This condition is provided to support in-circuit emulators.
         * When the emulator needs to access the debug registers, emulator software can set the GD flag to prevent interference
         * from the program currently executing on the processor.
         * The processor clears the GD flag upon entering to the debug exception handler, to allow the handler access to the debug
         * registers.
         */
        uint64_t general_detect : 1;
        uint64_t reserved3 : 2;

        /**
         * @brief R/W0 through R/W3 (read/write) fields (bits 16, 17, 20, 21, 24, 25, 28, and 29)
         *
         * [Bits 17:16] Specifies the breakpoint condition for the corresponding breakpoint. The DE (debug extensions) flag in
         * control register CR4 determines how the bits in the R/Wn fields are interpreted. When the DE flag is set, the processor
         * interprets bits as follows:
         * - 00 - Break on instruction execution only.
         * - 01 - Break on data writes only.
         * - 10 - Break on I/O reads or writes.
         * - 11 - Break on data reads or writes but not instruction fetches.
         * When the DE flag is clear, the processor interprets the R/Wn bits the same as for the Intel386(TM) and Intel486(TM)
         * processors, which is as follows:
         * - 00 - Break on instruction execution only.
         * - 01 - Break on data writes only.
         * - 10 - Undefined.
         * - 11 - Break on data reads or writes but not instruction fetches.
         */
        uint64_t read_write_0 : 2;

        /**
         * @brief LEN0 through LEN3 (Length) fields (bits 18, 19, 22, 23, 26, 27, 30, and 31)
         *
         * [Bits 19:18] Specify the size of the memory location at the address specified in the corresponding breakpoint address
         * register (DR0 through DR3). These fields are interpreted as follows:
         * - 00 - 1-byte length.
         * - 01 - 2-byte length.
         * - 10 - Undefined (or 8 byte length, see note below).
         * - 11 - 4-byte length.
         * If the corresponding RWn field in register DR7 is 00 (instruction execution), then the LENn field should also be 00. The
         * effect of using other lengths is undefined.
         *
         * @see Vol3B[17.2.5(Breakpoint Field Recognition)]
         */
        uint64_t length_0 : 2;
        uint64_t read_write_1 : 2;
        uint64_t length_1 : 2;
        uint64_t read_write_2 : 2;
        uint64_t length_2 : 2;
        uint64_t read_write_3 : 2;
        uint64_t length_3 : 2;
        uint64_t reserved4 : 32;
    };

    uint64_t flags;
} dr7;

/**
 * @}
 */