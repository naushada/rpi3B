/* Bare-metal image layout for QEMU `-M raspi3b` / real Pi 3 (aarch64 -kernel is
 * loaded at 0x80000). .text.boot must be first so _start is the entry. The
 * vector table's 2 KB alignment is enforced by `.balign 0x800` in vectors.S;
 * because .text starts at the 2 KB-aligned 0x80000, that alignment holds in the
 * final image. */
ENTRY(_start)

SECTIONS
{
    . = 0x80000;

    .text : {
        KEEP(*(.text.boot))
        *(.text.vectors)
        *(.text .text.*)
    }

    .rodata : { *(.rodata .rodata.*) }
    .data   : { *(.data .data.*) }

    . = ALIGN(16);
    __bss_start = .;
    .bss (NOLOAD) : { *(.bss .bss.*) *(COMMON) }
    . = ALIGN(16);
    __bss_end = .;

    /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) *(.interp) *(.dynamic) }
}
