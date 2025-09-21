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

#include "xsHost.h"

static int32_t gTimeZoneOffset = 0;
static int32_t gDaylightOffset = 0;
static int64_t gUnixTimeOffsetUS = 0;

static inline uint64_t modGetUptimeUS(void)
{
        return k_ticks_to_us_floor64(k_uptime_ticks());
}

static void modUpdateTimeVal(struct modTimeVal *tv, int64_t unixUS)
{
        if (!tv)
                return;
        if (unixUS < 0)
                unixUS = 0;
        tv->tv_sec = (modTime_t)(unixUS / 1000000);
        tv->tv_usec = (uint32_t)(unixUS % 1000000);
}

void ESP_putc(int c)
{
        printk("%c", c);
}

void modGetTimeOfDay(struct modTimeVal *tv, struct modTimeZone *tz)
{
        int64_t unixUS = (int64_t)modGetUptimeUS() + gUnixTimeOffsetUS;
        modUpdateTimeVal(tv, unixUS);
        if (tz) {
                tz->tz_minuteswest = -(gTimeZoneOffset / 60);
                tz->tz_dsttime = (gDaylightOffset != 0);
        }
}

static void modCopyNativeTm(struct modTm *result, const struct tm *native)
{
        result->tm_sec = native->tm_sec;
        result->tm_min = native->tm_min;
        result->tm_hour = native->tm_hour;
        result->tm_mday = native->tm_mday;
        result->tm_mon = native->tm_mon;
        result->tm_year = native->tm_year;
        result->tm_wday = native->tm_wday;
        result->tm_yday = native->tm_yday;
        result->tm_isdst = native->tm_isdst;
}

struct modTm *modGmTime(const modTime_t *timep)
{
        static struct modTm tm;
        time_t t = (time_t)*timep;
        struct tm native;
        gmtime_r(&t, &native);
        modCopyNativeTm(&tm, &native);
        return &tm;
}

struct modTm *modLocalTime(const modTime_t *timep)
{
        static struct modTm tm;
        time_t t = (time_t)(*timep + gTimeZoneOffset + gDaylightOffset);
        struct tm native;
        gmtime_r(&t, &native);
        native.tm_isdst = (gDaylightOffset != 0);
        modCopyNativeTm(&tm, &native);
        return &tm;
}

static int64_t modDaysFromCivil(int64_t year, unsigned month, unsigned day)
{
        year -= month <= 2;
        const int64_t era = (year >= 0 ? year : year - 399) / 400;
        const unsigned yoe = (unsigned)(year - era * 400);
        const unsigned doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
        return era * 146097 + (int64_t)doe - 719468;
}

static modTime_t modEpochFromTm(const struct modTm *tm)
{
        int64_t days = modDaysFromCivil((int64_t)tm->tm_year + 1900,
                                        (unsigned)tm->tm_mon + 1,
                                        (unsigned)tm->tm_mday);
        int64_t seconds = days * 86400LL + tm->tm_hour * 3600LL + tm->tm_min * 60LL + tm->tm_sec;
        if (seconds < 0)
                seconds = 0;
        return (modTime_t)seconds;
}

modTime_t modMkTime(struct modTm *tm)
{
        modTime_t value = modEpochFromTm(tm);
        int64_t adjust = (int64_t)gTimeZoneOffset + (int64_t)gDaylightOffset;
        if (adjust)
                value = (modTime_t)((int64_t)value - adjust);
        return value;
}

void modStrfTime(char *s, size_t max, const char *format, const struct modTm *tm)
{
        struct tm native = {
                .tm_sec = tm->tm_sec,
                .tm_min = tm->tm_min,
                .tm_hour = tm->tm_hour,
                .tm_mday = tm->tm_mday,
                .tm_mon = tm->tm_mon,
                .tm_year = tm->tm_year,
                .tm_wday = tm->tm_wday,
                .tm_yday = tm->tm_yday,
                .tm_isdst = tm->tm_isdst
        };
        strftime(s, max, format, &native);
}

void modSetTime(uint32_t seconds)
{
        gUnixTimeOffsetUS = ((int64_t)seconds * 1000000) - (int64_t)modGetUptimeUS();
}

int32_t modGetTimeZone(void)
{
        return gTimeZoneOffset;
}

void modSetTimeZone(int32_t offset)
{
        gTimeZoneOffset = offset;
}

int32_t modGetDaylightSavingsOffset(void)
{
        return gDaylightOffset;
}

void modSetDaylightSavingsOffset(int32_t offset)
{
        gDaylightOffset = offset;
}
