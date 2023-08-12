# Aquarius S3 BASIC ROM

By Curtis F. Kaylor

## Features / changes (with regards to S2 ROM)

### Universal (all versions)

- USR() defaults to executing the code at the passed in address, can still be changed by POKE. Useful when the CALL statement isnt available.
- Error routine patched to allow future command ON ERROR to restore the stack.
- FRCINT allows numbers between -65535 and 65535, so you can use positive instead of negative numbers with PEEK, POKE, etc.
- The width of columns when using a comma in print statements can be changed with a POKE. (S2: comma does columns 1 and 14)
- Added a hook to the operator evaluation routine, so extended basic can add new operators like XOR and MOD.
- Removed memory protection. You can POKE and PEEK any address.
- Changed the block transfer routine BLUTO to do an LDDR instead of using a loop.
- Modified the RND code to use the permutation table in ROM instead of the copy in RAM, freeing 32 bytes of system variables for extended BASIC to reuse.

### -Daqplus switch

- Patches entry points for aqplus_rom

### -Dnoreskeys

- Disables keyword expansion.
- Adds typing characters not on the original Aquarius keyboard such as: ```[ ] { } | ` ~```
