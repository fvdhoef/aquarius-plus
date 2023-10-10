    .globl  s__HEAP

    .area   _CODE
_getheap::
    ld      de, #s__HEAP
    ret
