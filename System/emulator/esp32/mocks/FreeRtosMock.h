#pragma once

#include <stdint.h>

#define ESP_LOGE(...) \
    do {              \
    } while (0)

#define ESP_LOGW(...) \
    do {              \
    } while (0)

#define ESP_LOGI(...) \
    do {              \
    } while (0)

#define configSTACK_DEPTH_TYPE uint32_t
#define portSTACK_TYPE         uint8_t
#define portBASE_TYPE          int
#define pdMS_TO_TICKS(x)       (x)
#define portMAX_DELAY          (TickType_t)0xffffffffUL
#define pdFALSE                ((BaseType_t)0)
#define pdTRUE                 ((BaseType_t)1)
#define pdPASS                 (pdTRUE)
#define pdFAIL                 (pdFALSE)

typedef uint32_t               TickType_t;
typedef portSTACK_TYPE         StackType_t;
typedef portBASE_TYPE          BaseType_t;
typedef unsigned portBASE_TYPE UBaseType_t;

void FreeRtosMock_init();
void FreeRtosMock_deinit();

//////////////////////////////////////////////////////////////////////////////
// Task
//////////////////////////////////////////////////////////////////////////////
typedef struct tskTaskControlBlock *TaskHandle_t;
typedef void (*TaskFunction_t)(void *);

BaseType_t xTaskCreate(
    TaskFunction_t               pxTaskCode,
    const char *const            pcName,
    const configSTACK_DEPTH_TYPE usStackDepth,
    void *const                  pvParameters,
    UBaseType_t                  uxPriority,
    TaskHandle_t *const          pxCreatedTask);

TickType_t xTaskGetTickCount();
void       vTaskDelay(const TickType_t xTicksToDelay);

//////////////////////////////////////////////////////////////////////////////
// Queue
//////////////////////////////////////////////////////////////////////////////
typedef struct QueueDefinition *QueueHandle_t;

QueueHandle_t xQueueCreate(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize);
BaseType_t    xQueueSend(QueueHandle_t xQueue, const void *const pvItemToQueue, TickType_t xTicksToWait);
BaseType_t    xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait);
BaseType_t    xQueueReset(QueueHandle_t xQueue);
UBaseType_t   uxQueueMessagesWaiting(const QueueHandle_t xQueue);

//////////////////////////////////////////////////////////////////////////////
// Semaphore
//////////////////////////////////////////////////////////////////////////////
typedef struct SemaphoreDefinition *SemaphoreHandle_t;

SemaphoreHandle_t xSemaphoreCreateRecursiveMutex();
void              vSemaphoreDelete(SemaphoreHandle_t xMutex);
BaseType_t        xSemaphoreTakeRecursive(SemaphoreHandle_t xMutex, TickType_t xTicksToWait);
BaseType_t        xSemaphoreGiveRecursive(SemaphoreHandle_t xMutex);

//////////////////////////////////////////////////////////////////////////////
// Timer
//////////////////////////////////////////////////////////////////////////////
typedef struct tmrTimerControl *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

TimerHandle_t xTimerCreate(
    const char *const       pcTimerName,
    const TickType_t        xTimerPeriodInTicks,
    const BaseType_t        xAutoReload,
    void *const             pvTimerID,
    TimerCallbackFunction_t pxCallbackFunction);

BaseType_t xTimerDelete(TimerHandle_t xTimer, TickType_t xTicksToWait);
void      *pvTimerGetTimerID(const TimerHandle_t xTimer);
BaseType_t xTimerStart(TimerHandle_t xTimer, TickType_t xTicksToWait);
BaseType_t xTimerStop(TimerHandle_t xTimer, TickType_t xTicksToWait);
BaseType_t xTimerReset(TimerHandle_t xTimer, TickType_t xTicksToWait);
