#pragma once
#include "FreeRTOS.h"

typedef void (*TaskFunction_t)(void*);

inline BaseType_t xTaskCreatePinnedToCore(TaskFunction_t, const char*, uint32_t, void*, UBaseType_t, TaskHandle_t*, UBaseType_t){ return pdTRUE; }
inline UBaseType_t uxTaskPriorityGet(TaskHandle_t){ return 0; }
inline BaseType_t xPortGetCoreID(){ return 0; }
inline void vTaskDelay(TickType_t) {}
