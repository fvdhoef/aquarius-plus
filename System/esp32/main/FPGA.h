#pragma once

#include "Common.h"
#include <driver/spi_master.h>

enum {
    IO_BANK0 = 0xF0,
    IO_BANK1 = 0xF1,
    IO_BANK2 = 0xF2,
    IO_BANK3 = 0xF3,
};

class FPGA {
    FPGA();

public:
    static FPGA &instance();

    void init();
    bool loadBitstream(const void *data, size_t length);
    bool loadDefaultBitstream();

    void    aqpReset();
    void    aqpUpdateKeybMatrix(uint64_t keyb_matrix);
    void    aqpUpdateHandCtrl(uint8_t hctrl1, uint8_t hctrl2, TickType_t ticks_to_wait = portMAX_DELAY);
    void    aqpWriteKeybBuffer(uint8_t ch);
    void    aqpAqcuireBus();
    void    aqpReleaseBus();
    void    aqpWriteMem(uint16_t addr, uint8_t data);
    uint8_t aqpReadMem(uint16_t addr);
    void    aqpWriteIO(uint16_t addr, uint8_t data);
    uint8_t aqpReadIO(uint16_t addr);
    void    aqpWriteROM(uint16_t addr, uint8_t data);
    void    aqpSaveMemBanks();
    void    aqpRestoreMemBanks();
    void    aqpSetMemBank(unsigned bank, uint8_t val);
    void    aqpSetVideoTimingMode(uint8_t mode);

private:
    void fpgaTx(const void *data, size_t length);
    void aqpSel(bool enable);
    void aqpTx(int data0 = -1, int data1 = -1, int data2 = -1, int data3 = -1);
    void aqpTx(const void *data, size_t length);
    void aqpRx(void *buf, size_t length);

    SemaphoreHandle_t   mutex;
    spi_device_handle_t fpgaSpiDev;
    spi_device_handle_t aqpSpiDev;

    uint8_t saved_banks[4];
    uint8_t cur_banks[4];
};
