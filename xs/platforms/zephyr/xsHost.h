/*
 * Copyright (c) 2016-2025  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 *
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __XSZEPHYR_HOST__
#define __XSZEPHYR_HOST__

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/byteorder.h>

#include <errno.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*modMessageDeliver)(void *the, void *refcon, uint8_t *message, uint16_t messageLength);

#define ICACHE_RAM_ATTR
#define ICACHE_FLASH_ATTR
#define ICACHE_FLASH1_ATTR
#define ICACHE_RODATA_ATTR
#define ICACHE_XS6RO_ATTR
#define ICACHE_XS6RO2_ATTR
#define ICACHE_XS6STRING_ATTR

void modLog_transmit(const char *msg);
void ESP_putc(int c);

#define modLog(msg) do { modLog_transmit(msg); } while (0)
#define modLogVar(msg) modLog_transmit(msg)
#define modLogInt(msg) do { char temp[12]; snprintf(temp, sizeof(temp), "%d", (int)(msg)); modLog_transmit(temp); } while (0)
#define modLogHex(msg) do { char temp[12]; snprintf(temp, sizeof(temp), "%x", (unsigned int)(msg)); modLog_transmit(temp); } while (0)
#define xmodLog(msg)
#define xmodLogVar(msg)
#define xmodLogInt(msg)
#define xmodLogHex(msg)

#define modDelayMilliseconds(ms) k_msleep(ms)
#define modDelayMicroseconds(us) k_busy_wait(us)
#define modMilliseconds() (uint32_t)k_uptime_get_32()
#define modMicroseconds() (uint32_t)k_ticks_to_us_floor64(k_uptime_ticks())

#define modCriticalSectionDeclare unsigned int _irq_key
#define modCriticalSectionBegin() do { _irq_key = irq_lock(); } while (0)
#define modCriticalSectionEnd() do { irq_unlock(_irq_key); } while (0)

typedef uint32_t modTime_t;

struct modTimeVal {
        modTime_t tv_sec;
        uint32_t tv_usec;
};
typedef struct modTimeVal modTimeVal;

struct modTimeZone {
        int32_t tz_minuteswest;
        int32_t tz_dsttime;
};

struct modTm {
        int32_t tm_sec;
        int32_t tm_min;
        int32_t tm_hour;
        int32_t tm_mday;
        int32_t tm_mon;
        int32_t tm_year;
        int32_t tm_wday;
        int32_t tm_yday;
        int32_t tm_isdst;
};
typedef struct modTm modTm;

void modGetTimeOfDay(struct modTimeVal *tv, struct modTimeZone *tz);
struct modTm *modGmTime(const modTime_t *timep);
struct modTm *modLocalTime(const modTime_t *timep);
modTime_t modMkTime(struct modTm *tm);
void modStrfTime(char *s, size_t max, const char *format, const struct modTm *tm);
void modSetTime(uint32_t seconds);
int32_t modGetTimeZone(void);
void modSetTimeZone(int32_t offset);
int32_t modGetDaylightSavingsOffset(void);
void modSetDaylightSavingsOffset(int32_t offset);

#define c_malloc malloc
#define c_calloc calloc
#define c_realloc realloc
#define c_free free
#define c_memcpy memcpy
#define c_memmove memmove
#define c_memset memset
#define c_memcmp memcmp
#define c_strlen strlen
#define c_strcmp strcmp
#define c_strncmp strncmp
#define c_strncpy strncpy
#define c_strchr strchr
#define c_strrchr strrchr
#define c_strstr strstr
#define c_strcspn strcspn
#define c_strspn strspn
#define c_strcpy strcpy
#define c_strcat strcat
#define c_sprintf sprintf
#define c_snprintf snprintf
#define c_vsnprintf vsnprintf
#define c_strtol strtol
#define c_strtoul strtoul
#define c_strtod strtod
#define c_fmod fmod
#define c_floor floor
#define c_ceil ceil
#define c_pow pow
#define c_log log
#define c_log10 log10
#define c_sin sin
#define c_cos cos
#define c_tan tan
#define c_asin asin
#define c_acos acos
#define c_atan atan
#define c_atan2 atan2
#define c_sqrt sqrt
#define c_abs abs
#define c_toupper toupper
#define c_tolower tolower
#define c_qsort qsort
#define c_modf modf
#define c_ldexp ldexp
#define c_isnan isnan
#define c_isfinite isfinite
#define c_isnormal isnormal
#define c_hypot hypot
#define c_rand rand
#define c_srand srand
#define c_time time

#define c_read8(POINTER) *((uint8_t*)(POINTER))
#define c_read16(POINTER) *((uint16_t*)(POINTER))
#define c_read32(POINTER) *((uint32_t*)(POINTER))
#define c_read16be(POINTER) sys_be16_to_cpu(*((uint16_t*)(POINTER)))
#define c_read32be(POINTER) sys_be32_to_cpu(*((uint32_t*)(POINTER)))

#define mxGetKeySlotID(SLOT) (SLOT)->ID
#define mxGetKeySlotKind(SLOT) (SLOT)->kind

#define modSPIFlashInit() (0)
#define modSPIRead(OFFSET, SIZE, DST) (0)
#define modSPIWrite(OFFSET, SIZE, SRC) (0)
#define modSPIErase(OFFSET, SIZE) (0)
#define modGetPartition(W, O, S) (0)

#include "xs.h"

void modMachineTaskInit(xsMachine *the);
void modMachineTaskUninit(xsMachine *the);
void modMachineTaskWait(xsMachine *the);
void modMachineTaskWake(xsMachine *the);

int modMessagePostToMachine(xsMachine *the, uint8_t *message, uint16_t messageLength, modMessageDeliver callback, void *refcon);
int modMessagePostToMachineFromISR(xsMachine *the, modMessageDeliver callback, void *refcon);
int modMessageService(xsMachine *the, int maxDelayMS);

#ifdef __cplusplus
}
#endif

#endif /* __XSZEPHYR_HOST__ */
