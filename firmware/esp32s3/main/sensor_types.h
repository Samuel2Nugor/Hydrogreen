#pragma once

#include <stdbool.h>

/* Shared result type for every sensor driver's read function. */
typedef struct {
    float value;
    bool  valid;
    char  status[16];   /* "ok" | "read_error" | "not_detected" */
} sensor_reading_t;
