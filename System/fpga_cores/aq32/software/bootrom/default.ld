MEMORY {
    rom (rx)  : ORIGIN = 0xFFFFF800, LENGTH = 0x00000800  /*   2k */
    ram (rwx) : ORIGIN = 0x00000000, LENGTH = 0x00080000  /* 512k */
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
    } > rom

    .rodata : ALIGN(8) {
        *(.rdata)
        *(.rodata)
        *(.rodata.*)
        *(.gnu.linkonce.r.*)
        *(.srodata.*)
    } > rom

    .data : ALIGN(8) {
        *(.data .data.*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = . + 0x800 );
        *(.sdata .sdata.* .sdata2.*)
        *(.gnu.linkonce.s.*)
    } > ram AT > rom

    .tdata : ALIGN(8) {
        PROVIDE( __tls_base = . );
	    *(.tdata .tdata.* .gnu.linkonce.td.*)
    } >ram AT >rom

    PROVIDE(_data_source_start = LOADADDR(.data));
    PROVIDE(_data_target_start = ADDR(.data));
    PROVIDE(_data_target_end   = ADDR(.tdata) + SIZEOF(.tdata));

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
