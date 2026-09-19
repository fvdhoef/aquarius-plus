#include "FreeRtosMock.h"
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdio.h>
#include <string.h>
#include <set>

static std::chrono::time_point startTime = std::chrono::steady_clock::now();

//////////////////////////////////////////////////////////////////////////////
// Task
//////////////////////////////////////////////////////////////////////////////
struct tskTaskControlBlock {
    std::thread t;
};

BaseType_t xTaskCreate(
    TaskFunction_t               pxTaskCode,
    const char *const            pcName,
    const configSTACK_DEPTH_TYPE usStackDepth,
    void *const                  pvParameters,
    UBaseType_t                  uxPriority,
    TaskHandle_t *const          pxCreatedTask) {

    auto tcb = new tskTaskControlBlock();
    tcb->t   = std::thread([pxTaskCode, pvParameters]() {
        pxTaskCode(pvParameters);
    });

    if (pxCreatedTask) {
        *pxCreatedTask = tcb;
    } else {
        tcb->t.detach();
        delete tcb;
    }
    return pdTRUE;
}

TickType_t xTaskGetTickCount() {
    auto now = std::chrono::steady_clock::now();
    return (TickType_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
}

void vTaskDelay(const TickType_t xTicksToDelay) {
    std::this_thread::sleep_for(std::chrono::milliseconds(xTicksToDelay));
}

//////////////////////////////////////////////////////////////////////////////
// Queue
//////////////////////////////////////////////////////////////////////////////
struct QueueDefinition {
    std::timed_mutex            mutex;
    std::condition_variable_any cvSend;
    std::condition_variable_any cvReceive;

    uint8_t    *buf;
    UBaseType_t length;
    UBaseType_t itemSize;
    unsigned    count;
    unsigned    wrIdx;
    unsigned    rdIdx;
};

QueueHandle_t xQueueCreate(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize) {
    auto qh      = new QueueDefinition();
    qh->buf      = new uint8_t[uxQueueLength * uxItemSize];
    qh->itemSize = uxItemSize;
    qh->length   = uxQueueLength;
    qh->count    = 0;
    qh->wrIdx    = 0;
    qh->rdIdx    = 0;
    return qh;
}

BaseType_t xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait) {
    auto tpUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(xTicksToWait);

    std::unique_lock lock(xQueue->mutex, std::defer_lock);
    while (1) {
        if (lock.try_lock_until(tpUntil)) {
            if (xQueue->count < xQueue->length) {
                // Copy item into queue
                memcpy(xQueue->buf + xQueue->itemSize * xQueue->wrIdx, pvItemToQueue, xQueue->itemSize);
                xQueue->count++;
                if (++xQueue->wrIdx >= xQueue->length)
                    xQueue->wrIdx = 0;

                xQueue->cvSend.notify_one();
                return pdTRUE;
            }

            if (!xQueue->cvReceive.wait_until(lock, tpUntil, [xQueue] { return xQueue->count < xQueue->length; })) {
                return pdFALSE;
            }

            lock.unlock();
        }
    }
    return pdFALSE;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait) {
    auto tpUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(xTicksToWait);

    std::unique_lock lock(xQueue->mutex, std::defer_lock);
    while (1) {
        if (lock.try_lock_until(tpUntil)) {
            if (xQueue->count > 0) {
                // Copy item from queue
                memcpy(pvBuffer, xQueue->buf + xQueue->itemSize * xQueue->rdIdx, xQueue->itemSize);
                xQueue->count--;
                if (++xQueue->rdIdx >= xQueue->length)
                    xQueue->rdIdx = 0;

                xQueue->cvReceive.notify_one();
                return pdTRUE;
            }

            if (!xQueue->cvSend.wait_until(lock, tpUntil, [xQueue] { return xQueue->count > 0; })) {
                return pdFALSE;
            }

            lock.unlock();
        }
    }
    return pdFALSE;
}

BaseType_t xQueueReset(QueueHandle_t xQueue) {
    std::lock_guard lock(xQueue->mutex);
    xQueue->count = 0;
    xQueue->wrIdx = 0;
    xQueue->rdIdx = 0;
    return pdTRUE;
}

UBaseType_t uxQueueMessagesWaiting(const QueueHandle_t xQueue) {
    std::lock_guard lock(xQueue->mutex);
    return xQueue->count;
}

//////////////////////////////////////////////////////////////////////////////
// Semaphore
//////////////////////////////////////////////////////////////////////////////
struct SemaphoreDefinition {
    std::recursive_timed_mutex mutex;
};

SemaphoreHandle_t xSemaphoreCreateRecursiveMutex() {
    auto sh = new SemaphoreDefinition();
    return sh;
}

BaseType_t xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t xTicksToWait) {
    return xMutex->mutex.try_lock_for(std::chrono::milliseconds(xTicksToWait)) ? pdTRUE : pdFALSE;
}

BaseType_t xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex) {
    xMutex->mutex.unlock();
    return pdTRUE;
}

void vSemaphoreDelete(SemaphoreHandle_t xMutex) {
    delete xMutex;
}

//////////////////////////////////////////////////////////////////////////////
// Timer
//////////////////////////////////////////////////////////////////////////////
struct tmrTimerControl {
    TickType_t                            period;
    bool                                  autoReload;
    void                                 *timerId;
    TimerCallbackFunction_t               func;
    std::chrono::steady_clock::time_point expiryTime;
};

static auto compTc = [](const tmrTimerControl *lhs, const tmrTimerControl *rhs) {
    return lhs->expiryTime < rhs->expiryTime;
};

static std::set<tmrTimerControl *, decltype(compTc)> activeTimers(compTc);
static std::timed_mutex                              activeTimersMutex;
static std::condition_variable_any                   activeTimersCv;
static bool                                          quit = false;
static std::thread                                   timerThread;

TimerHandle_t xTimerCreate(
    const char *const       pcTimerName,
    const TickType_t        xTimerPeriodInTicks,
    const BaseType_t        xAutoReload,
    void *const             pvTimerID,
    TimerCallbackFunction_t pxCallbackFunction) {

    auto tc        = new tmrTimerControl();
    tc->period     = xTimerPeriodInTicks;
    tc->autoReload = xAutoReload;
    tc->timerId    = pvTimerID;
    tc->func       = pxCallbackFunction;
    return tc;
}

BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    std::lock_guard lock(activeTimersMutex);
    activeTimers.erase(xTimer);
    delete xTimer;
    return pdTRUE;
}

void *pvTimerGetTimerID(const TimerHandle_t xTimer) {
    return xTimer->timerId;
}

BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    std::lock_guard lock(activeTimersMutex);
    activeTimers.erase(xTimer);

    xTimer->expiryTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(xTimer->period);
    activeTimers.insert(xTimer);

    activeTimersCv.notify_one();
    return pdTRUE;
}

BaseType_t xTimerStop(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    std::lock_guard lock(activeTimersMutex);
    activeTimers.erase(xTimer);

    activeTimersCv.notify_one();
    return pdTRUE;
}

BaseType_t xTimerReset(TimerHandle_t xTimer, TickType_t xTicksToWait) {
    std::lock_guard lock(activeTimersMutex);
    activeTimers.erase(xTimer);

    xTimer->expiryTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(xTimer->period);
    activeTimers.insert(xTimer);

    activeTimersCv.notify_one();
    return pdTRUE;
}

static void timerThreadFunc() {
    std::unique_lock lock(activeTimersMutex);
    while (!quit) {
        if (!activeTimers.empty()) {
            auto t = *activeTimers.begin();
            if (t->expiryTime <= std::chrono::steady_clock::now()) {
                activeTimers.erase(t);
                if (t->autoReload) {
                    t->expiryTime += std::chrono::milliseconds(t->period);
                    activeTimers.insert(t);
                }
                lock.unlock();
                t->func(t);
                lock.lock();
                continue;
            }

            activeTimersCv.wait_until(lock, t->expiryTime);

        } else {
            activeTimersCv.wait(lock);
        }
    }
}

void FreeRtosMock_init() {
    timerThread = std::thread([] { timerThreadFunc(); });

    // startTime = std::chrono::steady_clock::now();
}

void FreeRtosMock_deinit() {
    quit = true;
    activeTimersCv.notify_all();
    timerThread.join();

    // startTime = std::chrono::steady_clock::now();
}
