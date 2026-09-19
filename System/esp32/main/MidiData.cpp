#include "MidiData.h"

class MidiDataInt : public MidiData {
public:
    QueueHandle_t     dataQueue;
    SemaphoreHandle_t mutex;

    MidiDataInt() {
        mutex     = xSemaphoreCreateRecursiveMutex();
        dataQueue = xQueueCreate(256, 4);
    }

    bool getData(uint8_t buf[4]) override {
        RecursiveMutexLock lock(mutex);
        return xQueueReceive(dataQueue, buf, 0) == pdTRUE;
    }

    unsigned getDataCount() override {
        RecursiveMutexLock lock(mutex);
        return uxQueueMessagesWaiting(dataQueue);
    }

    void addData(const uint8_t buf[4]) override {
        RecursiveMutexLock lock(mutex);
        xQueueSend(dataQueue, buf, 0);
    }
};

MidiData *MidiData::instance() {
    static MidiDataInt obj;
    return &obj;
}
