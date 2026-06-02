ENTRY(main)

SECTIONS
{
    /* Start memory (example RAM region) */
    . = 0x20000000;

    /* Normal code */
    .text :
    {
        *(.text*)
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata*)
    }

    /* Initialized data */
    .data :
    {
        *(.data*)
    }

    /* Uninitialized data */
    .bss :
    {
        *(.bss*)
    }

    /* ⭐ FIXED ADDRESS SECTION */
    . = 0x20001000;   /* Force location */

    .my_fixed_section :
    {
        KEEP(*(.my_fixed_section))
    }
}
