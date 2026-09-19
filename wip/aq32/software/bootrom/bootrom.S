// Boot ROM
//
// Fully written in assembly to make sure no stack pointer is needed.

#define LOADADDR 0x80400

#define REG_ESP_CTRL 0x2000
#define REG_ESP_DATA 0x2004

#define ESPCMD_RESET    0x01
#define ESPCMD_OPEN     0x10
#define ESPCMD_READ     0x12
#define ESPCMD_CLOSEALL 0x1F

// Register usage
// a0 = filename to load
// t0 = register base
// t1 = temp reg
// t2 = string pointer during open file, load address during read file
// t3 = num bytes received
// t5 = file descriptor
// t6 = copy of original a0

    .section ".text.init"
    .global reset_handler
reset_handler:
    j       _reset_entry
    j       _load_program_entry

_reset_entry:
    // Disable interrupts
    csrw    mstatus, zero
    csrw    mie, zero

    // t0=register base
    li      t0, REG_ESP_CTRL

    // Reset ESP
    li      a0, ESPCMD_RESET
    call    _esp_cmd

    // Use default file name
    la      a0, _fn

_load_program_entry:
    // Disable interrupts
    csrw    mstatus, zero
    csrw    mie, zero

    // Save a0 in t6
    mv      t6, a0

    // t0=register base
    li      t0, REG_ESP_CTRL

    // Close all files
    li      a0, ESPCMD_CLOSEALL
    call    _esp_cmd
    call    _esp_get_byte

    // Open file
    li      a0, ESPCMD_OPEN
    call    _esp_cmd
    li      a0, 0
    call    _esp_send_byte
    mv      t2, t6
1:  lbu     a0, (t2)
    addi    t2, t2, 1
    call    _esp_send_byte
    bnez    a0, 1b
    call    _esp_get_byte
    sext.b  a0, a0
    bltz    a0, _halt
    mv      t5, a0

    // Read file into memory
    li      t2, LOADADDR

2:  li      a0, ESPCMD_READ     // Command
    call    _esp_cmd
    mv      a0, t5              // File descriptor
    call    _esp_send_byte
    li      a0, 0               // Read amount (0x8000)
    call    _esp_send_byte
    li      a0, 0x80
    call    _esp_send_byte
    call    _esp_get_byte       // Get status, halt on error
    sext.b  a0, a0
    bltz    a0, _halt
    call    _esp_get_byte       // Get amount in result
    mv      t3, a0
    call    _esp_get_byte
    slli    a0, a0, 8
    or      t3, t3, a0
    beqz    t3, 4f              // Done if zero

3:  call    _esp_get_byte       // Read bytes into memory
    sb      a0, (t2)
    addi    t2, t2, 1
    addi    t3, t3, -1
    bnez    t3, 3b
    j       2b

4:
    // Close all files
    li      a0, ESPCMD_CLOSEALL
    call    _esp_cmd
    call    _esp_get_byte

    // Jump to entry point
    mv      a0, t6              // Restore original a0
    j       LOADADDR

_halt:
    j       _halt

_esp_cmd:     // cmd in a0, clobbers t1
    // Drain RX FIFO
    lw      t1, 0(t0)
    andi    t1, t1, 1
    beqz    t1, 1f
    lw      t1, 4(t0)
    j       _esp_cmd
1:
    // Wait while busy
    lw      t1, 0(t0)
    andi    t1, t1, 2
    bnez    t1, 1b

    // Start of message
    li      t1, 0x100
    sw      t1, 4(t0)

    // fall into _esp_send_byte

_esp_send_byte:     // value in a0, clobbers t1
    // Wait while busy
    lw      t1, 0(t0)
    andi    t1, t1, 2
    bnez    t1, _esp_send_byte

    // Send byte
    sw      a0, 4(t0)
    ret

_esp_get_byte:      // result in a0, clobbers t1
    // Wait for byte
    lw      t1, 0(t0)
    andi    t1, t1, 1
    beqz    t1, _esp_get_byte

    lw      a0, 4(t0)
    ret

_fn: .ascii "/cores/aq32/boot.aq32\0"
