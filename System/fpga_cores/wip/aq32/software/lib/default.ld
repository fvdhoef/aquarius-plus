STACK_SIZE = 8k;

MEMORY {
    ram   (rwx) : ORIGIN = 0x80400,                   LENGTH = 512k - 1k - STACK_SIZE
    stack (rw)  : ORIGIN = ORIGIN(ram) + LENGTH(ram), LENGTH = STACK_SIZE
}
PROVIDE(__stack_start = ORIGIN(stack));
PROVIDE(__stack_end   = ORIGIN(stack) + LENGTH(stack));

ENTRY(_start)

PHDRS {
  text   PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data   PT_LOAD FLAGS(6);
  bss    PT_LOAD FLAGS(6);
}

SECTIONS {
    /DISCARD/ : {
     /* *(*.data._impure_ptr) */
        *(*.riscv.attributes)
        *(.comment)
     /* *(.debug_*) */
    }

    .text : ALIGN(16) {
        *(.text.init)
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
        . = ALIGN(16);

        KEEP(*crtbegin.o(.ctors))
        KEEP(*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP(*(SORT(.ctors.*)))
        KEEP(*crtend.o(.ctors))
        KEEP(*crtbegin.o(.dtors))
        KEEP(*(EXCLUDE_FILE (*crtend.o) .dtors))
        KEEP(*(SORT(.dtors.*)))
        KEEP(*crtend.o(.dtors))

        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)

        KEEP(*(.init))
        KEEP(*(.fini))

        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN(__preinit_array_end = .);
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN(__init_array_end = .);
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(.fini_array))
        KEEP(*(SORT(.fini_array.*)))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > ram :text

    .rodata : ALIGN(16) {
        *(.rdata)
        *(.rodata .rodata.*)
        *(.gnu.linkonce.r.*)
        *(.srodata.*)
    } > ram :rodata

    .data : ALIGN(16) {
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(16);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata2.*)
        *(.gnu.linkonce.s.*)
        . = ALIGN(16);
    } > ram :data

    .tdata : ALIGN(16) {
        PROVIDE( __tls_base = . );
	    *(.tdata .tdata.* .gnu.linkonce.td.*)
    } > ram :data

    .bss : ALIGN(16) {
        PROVIDE(__bss_start = .);
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(16);
        PROVIDE(__bss_end = .);
    } > ram :bss

    .noinit (NOLOAD) : ALIGN(16) {
        KEEP(*(*.noinit*))
    } > ram :bss

    _end = .;
}
