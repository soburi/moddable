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

void ESP_putc(int c)
{
        printk("%c", c);
}

void modGetTimeOfDay(struct modTimeVal *tv, struct modTimeZone *tz)
{
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        if (tv) {
                tv->tv_sec = (modTime_t)ts.tv_sec;
                tv->tv_usec = (uint32_t)(ts.tv_nsec / 1000);
        }
        if (tz) {
                tz->tz_minuteswest = -(gTimeZoneOffset / 60);
                tz->tz_dsttime = (gDaylightOffset != 0);
        }
}

struct modTm *modGmTime(const modTime_t *timep)
{
        static struct modTm tm;
        time_t t = (time_t)*timep;
        struct tm result;
        gmtime_r(&t, &result);
        tm.tm_sec = result.tm_sec;
        tm.tm_min = result.tm_min;
        tm.tm_hour = result.tm_hour;
        tm.tm_mday = result.tm_mday;
        tm.tm_mon = result.tm_mon;
        tm.tm_year = result.tm_year;
        tm.tm_wday = result.tm_wday;
        tm.tm_yday = result.tm_yday;
        tm.tm_isdst = result.tm_isdst;
        return &tm;
}

struct modTm *modLocalTime(const modTime_t *timep)
{
        static struct modTm tm;
        time_t t = (time_t)*timep;
        struct tm result;
        localtime_r(&t, &result);
        tm.tm_sec = result.tm_sec;
        tm.tm_min = result.tm_min;
        tm.tm_hour = result.tm_hour;
        tm.tm_mday = result.tm_mday;
        tm.tm_mon = result.tm_mon;
        tm.tm_year = result.tm_year;
        tm.tm_wday = result.tm_wday;
        tm.tm_yday = result.tm_yday;
        tm.tm_isdst = result.tm_isdst;
        return &tm;
}

modTime_t modMkTime(struct modTm *tm)
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
        time_t value = mktime(&native);
        return (modTime_t)value;
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
        struct timespec ts = {
                .tv_sec = seconds,
                .tv_nsec = 0
        };
        clock_settime(CLOCK_REALTIME, &ts);
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
