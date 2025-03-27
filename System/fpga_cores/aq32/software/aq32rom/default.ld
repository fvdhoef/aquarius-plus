MEMORY {
    ram (rwx) : ORIGIN = 0x80000000, LENGTH = 0x00080000  /* 512k */
}

PROVIDE(__ram_end = ORIGIN(ram) + LENGTH(ram));

ENTRY(reset_handler)

SECTIONS {
    /DISCARD/ : {
        *(*.data._impure_ptr)
        *(*.riscv.attributes)
        *(.comment)
/*        *(.debug_*) */
    }

    .text : {
        *(.text.init)
        *(.text.unlikely .text.unlikely.*)
        *(.text.startup .text.startup.*)
        *(.text .text.*)
        *(.gnu.linkonce.t.*)
    } > ram

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        *(.srodata.*)
    } > ram

    .data : ALIGN(8) {
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata2.*)
        *(.gnu.linkonce.s.*)
    } > ram

    .tdata : ALIGN(8) {
        PROVIDE( __tls_base = . );
	    *(.tdata .tdata.* .gnu.linkonce.td.*)
    } > ram

    .bss : ALIGN(8) {
        *(.sbss*)
        *(.gnu.linkonce.sb.*)
        *(.bss .bss.*)
        *(.gnu.linkonce.b.*)
        *(COMMON)
    } > ram

    PROVIDE(_bss_source_start = LOADADDR(.bss));
    PROVIDE(_bss_target_start = ADDR(.bss));
    PROVIDE(_bss_target_end   = ADDR(.bss) + SIZEOF(.bss));

    . = ALIGN(8);
    _end = . ;
}
