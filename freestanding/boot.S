/*
 * boot.S — freestanding aarch64 entry for the BCM2837 (Raspberry Pi 3B).
 *
 * QEMU `-M raspi3b -kernel kernel8.img` loads this at 0x80000 and starts all
 * four cores here. We park the secondary cores, drop to EL1 (the Pi's GPU
 * firmware / armstub normally hands the kernel over at EL2), grant EL1 access to
 * the physical generic timer, set up a stack, clear .bss, and call kmain().
 *
 * This is the runtime that makes inc/interrupt.hpp's AArch64 VBAR_EL1 model
 * real: kmain installs the vector table (vectors.S) via IRQ::install_vector_table
 * and takes a generic-timer IRQ through IRQ::dispatch().
 */

.section ".text.boot"
.global _start

_start:
    /* Only core 0 runs the kernel; cores 1..3 spin low-power. */
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF
    cbnz    x0, park_core

    /* Provisional stack for the drop code (top of the image load address). */
    ldr     x0, =_start
    mov     sp, x0

    /* Dispatch on the Exception Level we booted at. */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #3
    b.eq    from_el3
    cmp     x0, #2
    b.eq    from_el2
    b       el1_entry              /* already at EL1 */

from_el3:
    /* EL2/EL1 execution state is AArch64; non-secure. */
    mrs     x0, scr_el3
    orr     x0, x0, #(1 << 10)     /* RW: lower ELs are AArch64            */
    orr     x0, x0, #(1 << 0)      /* NS: non-secure                       */
    msr     scr_el3, x0
    mov     x0, #0x3c9             /* SPSR: EL2h, DAIF masked              */
    msr     spsr_el3, x0
    adr     x0, from_el2
    msr     elr_el3, x0
    eret

from_el2:
    /* Let EL1 read/use the physical counter & timer (CNTP*). */
    mrs     x0, cnthctl_el2
    orr     x0, x0, #(1 << 0)      /* EL1PCTEN: EL1 physical counter       */
    orr     x0, x0, #(1 << 1)      /* EL1PCEN:  EL1 physical timer regs    */
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    /* EL1 is AArch64. */
    mov     x0, #(1 << 31)
    msr     hcr_el2, x0

    /* Return into EL1h with interrupts masked; kmain unmasks. */
    mov     x0, #0x3c5             /* SPSR: EL1h, DAIF masked              */
    msr     spsr_el2, x0
    ldr     x0, =_start
    msr     sp_el1, x0
    adr     x0, el1_entry
    msr     elr_el2, x0
    eret

el1_entry:
    /* Solid EL1 stack (SP_EL1 may already be set from the EL2 path). */
    ldr     x0, =_start
    mov     sp, x0

    /* Zero .bss. */
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
1:  cmp     x0, x1
    b.hs    2f
    str     xzr, [x0], #8
    b       1b
2:
    bl      kmain

park_core:
    wfe
    b       park_core
