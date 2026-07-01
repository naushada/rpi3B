/*
 * boot.S — freestanding aarch64 entry for the BCM2837 (Raspberry Pi 3B).
 *
 * QEMU `-M raspi3b -kernel kernel8.img` loads this at 0x80000. Each core that
 * arrives here gets its own stack, drops to EL1 (the GPU firmware / armstub
 * normally hands over at EL2), grants EL1 access to the physical generic timer,
 * and branches: core 0 → kmain (full init), cores 1..3 → ksecondary.
 *
 * Per-core stacks are carved below the image: core N's stack top is
 * _start - N*STACK_SIZE, so they never overlap. Only core 0 clears .bss.
 */

.equ STACK_SIZE, 0x10000

.section ".text.boot"
.global _start

_start:
    /* x19 = core id (0..3); preserved across the EL drop. */
    mrs     x19, mpidr_el1
    and     x19, x19, #0xFF

    /* x22 = per-core stack top = _start - core*STACK_SIZE. */
    mov     x20, #STACK_SIZE
    mul     x21, x19, x20
    ldr     x22, =_start
    sub     x22, x22, x21
    mov     sp, x22

    /* Dispatch on the Exception Level we booted at. */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #3
    b.eq    from_el3
    cmp     x0, #2
    b.eq    from_el2
    b       el1_entry              /* already at EL1 */

from_el3:
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
    orr     x0, x0, #(1 << 0)      /* EL1PCTEN                             */
    orr     x0, x0, #(1 << 1)      /* EL1PCEN                              */
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr

    mov     x0, #(1 << 31)         /* HCR_EL2.RW: EL1 is AArch64           */
    msr     hcr_el2, x0

    mov     x0, #0x3c5             /* SPSR: EL1h, DAIF masked              */
    msr     spsr_el2, x0
    msr     sp_el1, x22            /* per-core EL1 stack                   */
    adr     x0, el1_entry
    msr     elr_el2, x0
    eret

el1_entry:
    mov     sp, x22                /* solid EL1 stack (per core)           */

    /* Only core 0 clears .bss — secondaries must not wipe shared state. */
    cbnz    x19, secondary

    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
1:  cmp     x0, x1
    b.hs    2f
    str     xzr, [x0], #8
    b       1b
2:
    bl      kmain
    b       park

secondary:
    bl      ksecondary

park:
    wfe
    b       park
