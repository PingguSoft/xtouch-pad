#ifndef _XTOUCH_DEBUG_
#define _XTOUCH_DEBUG_

#include <Arduino.h>

/*
*****************************************************************************************
* FEATURES
*****************************************************************************************
*/
#define _NO_REMOTE_PRINTER_ 1
#define _FTPS_RESULT_ONLY_  1

/*
*****************************************************************************************
* MACROS & STRUCTURES
*****************************************************************************************
*/
#define ARRAY_SIZE(x)                   (sizeof(x) / sizeof((x)[0]))
#define IS_ELAPSED(ts, last, duration)  ((ts) - (last) > (duration))

#define LOG_LEVEL_V             5
#define LOG_LEVEL_D             4
#define LOG_LEVEL_I             3
#define LOG_LEVEL_W             2
#define LOG_LEVEL_E             1
#define LOG_LEVEL_NONE          0

#ifndef XTOUCH_LOG_LEVEL
#define XTOUCH_LOG_LEVEL        LOG_LEVEL_V
#endif

#define LOG_FMT(level, format) "[%6u][" #level "][%30s:%6u] %30s(): " format, (unsigned long) (esp_timer_get_time() / 1000ULL), pathToFileName(__FILE__), __LINE__, __FUNCTION__

#if XTOUCH_LOG_LEVEL >= LOG_LEVEL_V
#define LOGV(format, ...)        printf(LOG_FMT(V, format), ##__VA_ARGS__)
#else
#define LOGV(format, ...)
#endif

#if XTOUCH_LOG_LEVEL >= LOG_LEVEL_D
#define LOGD(format, ...)        printf(LOG_FMT(D, format), ##__VA_ARGS__)
#else
#define LOGD(format, ...)
#endif

#if XTOUCH_LOG_LEVEL >= LOG_LEVEL_I
#define LOGI(format, ...)        printf(LOG_FMT(I, format), ##__VA_ARGS__)
#else
#define LOGI(format, ...)
#endif

#if XTOUCH_LOG_LEVEL >= LOG_LEVEL_W
#define LOGW(format, ...)        printf(LOG_FMT(W, format), ##__VA_ARGS__)
#else
#define LOGW(format, ...)
#endif

#if XTOUCH_LOG_LEVEL >= LOG_LEVEL_E
#define LOGE(format, ...)        printf(LOG_FMT(E, format), ##__VA_ARGS__)
#else
#define LOGE(format, ...)
#endif

#endif
