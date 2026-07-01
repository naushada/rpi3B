/*
 * vectors.S — the AArch64 (ARMv8-A) exception vector table this project models
 * in inc/interrupt.hpp. VBAR_EL1 points here (set by IRQ::install_vector_table).
 *
 * 16 entries, each a 0x80-byte slot, 2 KB-aligned, in four groups of four:
 *   {Current EL SP0} {Current EL SPx} {Lower EL AArch64} {Lower EL AArch32}
 * each group = {Synchronous, IRQ, FIQ, SError}. A kernel taking its own IRQ
 * while on SP_EL1 lands in the Current-EL-SPx IRQ slot (offset 0x280) — the slot
 * IVT::Number::IRQ maps to.
 *
 * Each slot branches to a stub that saves the integer context, calls a C handler,
 * restores, and ERETs. FP/SIMD is not saved: the C runtime is built
 * -mgeneral-regs-only.
 */

/* Save x0..x30 + ELR_EL1 + SPSR_EL1 (272 bytes, 16-aligned). */
.macro kernel_entry
    sub     sp, sp, #272
    stp     x0,  x1,  [sp, #16 * 0]
    stp     x2,  x3,  [sp, #16 * 1]
    stp     x4,  x5,  [sp, #16 * 2]
    stp     x6,  x7,  [sp, #16 * 3]
    stp     x8,  x9,  [sp, #16 * 4]
    stp     x10, x11, [sp, #16 * 5]
    stp     x12, x13, [sp, #16 * 6]
    stp     x14, x15, [sp, #16 * 7]
    stp     x16, x17, [sp, #16 * 8]
    stp     x18, x19, [sp, #16 * 9]
    stp     x20, x21, [sp, #16 * 10]
    stp     x22, x23, [sp, #16 * 11]
    stp     x24, x25, [sp, #16 * 12]
    stp     x26, x27, [sp, #16 * 13]
    stp     x28, x29, [sp, #16 * 14]
    mrs     x0,  elr_el1
    stp     x30, x0,  [sp, #16 * 15]
    mrs     x1,  spsr_el1
    str     x1,  [sp, #16 * 16]
.endm

.macro kernel_exit
    ldr     x1,  [sp, #16 * 16]
    msr     spsr_el1, x1
    ldp     x30, x0,  [sp, #16 * 15]
    msr     elr_el1, x0
    ldp     x2,  x3,  [sp, #16 * 1]
    ldp     x4,  x5,  [sp, #16 * 2]
    ldp     x6,  x7,  [sp, #16 * 3]
    ldp     x8,  x9,  [sp, #16 * 4]
    ldp     x10, x11, [sp, #16 * 5]
    ldp     x12, x13, [sp, #16 * 6]
    ldp     x14, x15, [sp, #16 * 7]
    ldp     x16, x17, [sp, #16 * 8]
    ldp     x18, x19, [sp, #16 * 9]
    ldp     x20, x21, [sp, #16 * 10]
    ldp     x22, x23, [sp, #16 * 11]
    ldp     x24, x25, [sp, #16 * 12]
    ldp     x26, x27, [sp, #16 * 13]
    ldp     x28, x29, [sp, #16 * 14]
    ldp     x0,  x1,  [sp, #16 * 0]
    add     sp, sp, #272
    eret
.endm

/* One 0x80-byte vector slot: branch to \label. \kind is passed to the bad-vector
 * handler so a crash report can say which one fired. */
.macro ventry label
    .balign 0x80
    b       \label
.endm

.section ".text.vectors"
.balign 0x800
.global vector_table
vector_table:
    /* Current EL with SP0 (unused: the kernel runs on SPx). */
    ventry  bad_sync            /* 0x000 Synchronous */
    ventry  bad_irq             /* 0x080 IRQ         */
    ventry  bad_fiq             /* 0x100 FIQ         */
    ventry  bad_serror          /* 0x180 SError      */

    /* Current EL with SPx (where the kernel takes its own exceptions). */
    ventry  bad_sync            /* 0x200 Synchronous */
    ventry  el1_irq             /* 0x280 IRQ  <-- the one we service */
    ventry  bad_fiq             /* 0x300 FIQ         */
    ventry  bad_serror          /* 0x380 SError      */

    /* Lower EL, AArch64 (no user tasks in this demo). */
    ventry  bad_sync            /* 0x400 */
    ventry  bad_irq             /* 0x480 */
    ventry  bad_fiq             /* 0x500 */
    ventry  bad_serror          /* 0x580 */

    /* Lower EL, AArch32 (unused). */
    ventry  bad_sync            /* 0x600 */
    ventry  bad_irq             /* 0x680 */
    ventry  bad_fiq             /* 0x700 */
    ventry  bad_serror          /* 0x780 */

el1_irq:
    kernel_entry
    bl      irq_handler_c
    kernel_exit

/* Unhandled vectors: report kind (x0) + ESR/ELR and hang inside bad_exception_c. */
bad_sync:
    kernel_entry
    mov     x0, #0
    b       report
bad_irq:
    kernel_entry
    mov     x0, #1
    b       report
bad_fiq:
    kernel_entry
    mov     x0, #2
    b       report
bad_serror:
    kernel_entry
    mov     x0, #3
report:
    mrs     x1, esr_el1
    mrs     x2, elr_el1
    bl      bad_exception_c
    kernel_exit
