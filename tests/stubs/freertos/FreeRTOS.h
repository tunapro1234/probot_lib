#pragma once
#include <stdint.h>

typedef void* QueueHandle_t;
typedef void* TaskHandle_t;
typedef uint32_t TickType_t;
typedef unsigned BaseType_t;
typedef unsigned UBaseType_t;

#define pdTRUE 1
#define pdFALSE 0

inline TickType_t pdMS_TO_TICKS(uint32_t ms){ return ms; }
