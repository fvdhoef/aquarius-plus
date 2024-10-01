#include "MemDump.h"
#include "VFS.h"
#include "FPGA.h"

uint8_t MemDump::savedRam3000[40];
uint8_t MemDump::savedRam3400[40];
int     MemDump::screenshotIdx = 1;

void MemDump::saveMsgRam() {
    // Save screen RAM where we put text
    auto fpga = getFPGA();
    for (int i = 0; i < 40; i++) {
        savedRam3000[i] = fpga->aqpReadMem(0x3000 + i);
        savedRam3400[i] = fpga->aqpReadMem(0x3400 + i);
    }
}

void MemDump::restoreMsgRam() {
    // Restore screen RAM
    auto fpga = getFPGA();
    for (int i = 0; i < 40; i++) {
        fpga->aqpWriteMem(0x3000 + i, savedRam3000[i]);
        fpga->aqpWriteMem(0x3400 + i, savedRam3400[i]);
    }
}

void MemDump::showMessage(const char *fmt, ...) {
    char msg[40];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    auto fpga = getFPGA();

    int         i = 1;
    const char *p = msg;
    for (; *p != 0; p++, i++) {
        fpga->aqpWriteMem(0x3000 + i, *p);
        fpga->aqpWriteMem(0x3400 + i, 0x70);
    }
}

void MemDump::dumpCartridge() {
    size_t      size     = 16384;
    const char *filename = "dumpfile.rom";

    printf("Dumping cartridge ROM to %s\n", filename);

    uint8_t *buf = (uint8_t *)malloc(size);
    if (!buf)
        return;

    auto fpga = getFPGA();
    fpga->aqpAqcuireBus();
    fpga->aqpSaveMemBanks();

    // Configure memory layout
    fpga->aqpWriteIO(IO_BANK0, (3 << 6) | 0);
    fpga->aqpWriteIO(IO_BANK3, 19);

    saveMsgRam();
    showMessage(" Saving cartridge to %s ", filename);

    // Read cartridge contents
    for (int i = 0; i < size; i++) {
        buf[i] = fpga->aqpReadMem(0xC000 + i);
    }

    // Save cartridge contents to file
    auto vfs = getSDCardVFS();
    int  fd;
    if ((fd = vfs->open(FO_WRONLY | FO_CREATE, filename)) >= 0) {
        vfs->write(fd, size, buf);
        vfs->close(fd);
    }

    restoreMsgRam();
    fpga->aqpRestoreMemBanks();
    fpga->aqpReleaseBus();
    free(buf);

    printf("Done!\n");
}

void MemDump::dumpScreen() {
    unsigned size = 2048;

    char filename[32];
    snprintf(filename, sizeof(filename), "screenshot%02u.scr", screenshotIdx++);

    printf("Dumping screen to %s\n", filename);

    uint8_t *buf = (uint8_t *)malloc(size);
    if (!buf)
        return;

    auto fpga = getFPGA();
    fpga->aqpAqcuireBus();
    fpga->aqpSaveMemBanks();

    // Configure memory layout
    fpga->aqpWriteIO(IO_BANK0, (3 << 6) | 0);

    // Save screen RAM where we put text
    saveMsgRam();
    showMessage(" Saving screen to %s ", filename);

    // Read memory
    memcpy(buf, savedRam3000, 40);
    memcpy(buf + 1024, savedRam3400, 40);
    for (int i = 40; i < 1024; i++) {
        buf[i]        = fpga->aqpReadMem(0x3000 + i);
        buf[i + 1024] = fpga->aqpReadMem(0x3400 + i);
    }

    // Save cartridge contents to file
    auto vfs = getSDCardVFS;
    int  fd;
    if ((fd = vfs->open(FO_WRONLY | FO_CREATE, filename)) >= 0) {
        vfs->write(fd, size, buf);
        vfs->close(fd);
    }

    // Restore screen RAM
    restoreMsgRam();

    fpga->aqpRestoreMemBanks();
    fpga->aqpReleaseBus();
    free(buf);

    printf("Done!\n");
}
