OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(SystemStartup)

KERNEL_VMA = 0xffffffff80000000;

PHDRS
{
    text PT_LOAD FLAGS((1 << 0) | (1 << 2));
    rodata PT_LOAD FLAGS((1 << 2));
    data PT_LOAD FLAGS((1 << 1) | (1 << 2));
}

SECTIONS
{
    . = KERNEL_VMA;
    __KernelStart = .;

    .text : {
        __TextStart = .;
        *(.text .text.*)
        __TextEnd = .;
    } :text

    .limine_requests : {
        KEEP(*(.limine_requests .limine_requests.*))
    } :text

    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        __RodataStart = .;
        *(.rodata .rodata.*)
        __RodataEnd = .;
    } :rodata

    . += CONSTANT(MAXPAGESIZE);

    .data : {
        __DataStart = .;
        *(.data .data.*)
        __DataEnd = .;
    } :data

    .bss : {
        __BssStart = .;
        *(COMMON)
        *(.bss .bss.*)
        __BssEnd = .;
    } :data

    __KernelEnd = .;
    __KernelSizeBytes = __KernelEnd - __KernelStart;
    __KernelSizePages = (__KernelSizeBytes + 0xfff) / 0x1000;

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
        *(.comment)
    }
}