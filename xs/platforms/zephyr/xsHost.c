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
 * This file incorporates work covered by the following copyright and  
 * permission notice:  
 *
 *       Copyright (C) 2010-2016 Marvell International Ltd.
 *       Copyright (C) 2002-2010 Kinoma, Inc.
 *
 *       Licensed under the Apache License, Version 2.0 (the "License");
 *       you may not use this file except in compliance with the License.
 *       You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *       Unless required by applicable law or agreed to in writing, software
 *       distributed under the License is distributed on an "AS IS" BASIS,
 *       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *       See the License for the specific language governing permissions and
 *       limitations under the License.
 */

#include "xsAll.h"
#include "xs.h"
#include "xsScript.h"
#include "xsPlatform.h"
#include "mc.defines.h"

#include "xsHosts.h"
#include "xsHost.h"

extern int modMessagePostToMachine(txMachine *the, uint8_t *message, uint16_t messageLength, modMessageDeliver callback, void *refcon);
extern void fxFreeZephyrMessage(txZephyrMessage *msg);

#if defined(CONFIG_NEWLIB_LIBC)
#include <malloc.h>
#endif

#ifdef mxInstrument
	#include "modTimer.h"
	#include "modInstrumentation.h"

	static void espInitInstrumentation(txMachine *the);
	static void espSampleInstrumentation(modTimer timer, void *refcon, int refconSize);

	#define espInstrumentCount kModInstrumentationSlotHeapSize - kModInstrumentationPixelsDrawn
	static char* const espInstrumentNames[espInstrumentCount] ICACHE_XS6RO_ATTR = {
		(char *)"Pixels drawn",
		(char *)"Frames drawn",
		(char *)"Network bytes read",
		(char *)"Network bytes written",
		(char *)"Network sockets",
		(char *)"Timers",
		(char *)"Files",
		(char *)"Poco display list used",
		(char *)"Piu command List used",
		(char *)"Event loop",
		(char *)"System bytes free",
	};

	static char* const espInstrumentUnits[espInstrumentCount] ICACHE_XS6RO_ATTR = {
		(char *)" pixels",
		(char *)" frames",
		(char *)" bytes",
		(char *)" bytes",
		(char *)" sockets",
		(char *)" timers",
		(char *)" files",
		(char *)" bytes",
		(char *)" bytes",
		(char *)" turns",
		(char *)" bytes"
	};
#endif

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

// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_15
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

void modSetTimeZone(int32_t offset)
{
	gTimeZoneOffset = offset;
}

int32_t modGetTimeZone(void)
{
	return gTimeZoneOffset;
}

void modSetDaylightSavingsOffset(int32_t offset)
{
	gDaylightOffset = offset;
}

int32_t modGetDaylightSavingsOffset(void)
{
	return gDaylightOffset;
}


/*
	Instrumentation
*/

#ifdef mxInstrument

void modInstrumentationSetup(xsMachine *the)
{
	espInitInstrumentation(the);
	modInstrumentMachineBegin(the, espSampleInstrumentation, espInstrumentCount, (char**)espInstrumentNames, (char**)espInstrumentUnits);
}

static int32_t modInstrumentationSystemFreeMemory(void *theIn)
{
        (void)theIn;
#if defined(CONFIG_NEWLIB_LIBC)
        struct mallinfo info = mallinfo();
        return (int32_t)info.fordblks;
#else
        return 0;
#endif
}


void espInitInstrumentation(txMachine *the)
{
#if MODDEF_XS_TEST
	static uint8_t initialized = 0;
	if (initialized)
		return;
	initialized = 1;
#endif

	modInstrumentationInit();
	modInstrumentationSetCallback(SystemFreeMemory, (ModInstrumentationGetter)modInstrumentationSystemFreeMemory);

	modInstrumentationSetCallback(SlotHeapSize, (ModInstrumentationGetter)modInstrumentationSlotHeapSize);
	modInstrumentationSetCallback(ChunkHeapSize, (ModInstrumentationGetter)modInstrumentationChunkHeapSize);
	modInstrumentationSetCallback(KeysUsed, (ModInstrumentationGetter)modInstrumentationKeysUsed);
	modInstrumentationSetCallback(GarbageCollectionCount, (ModInstrumentationGetter)modInstrumentationGarbageCollectionCount);
	modInstrumentationSetCallback(ModulesLoaded, (ModInstrumentationGetter)modInstrumentationModulesLoaded);
	modInstrumentationSetCallback(StackRemain, (ModInstrumentationGetter)modInstrumentationStackRemain);
	modInstrumentationSetCallback(PromisesSettledCount, (ModInstrumentationGetter)modInstrumentationPromisesSettledCount);
}

void espSampleInstrumentation(modTimer timer, void *refcon, int refconSize)
{
	txInteger values[espInstrumentCount];
	int what;
	xsMachine *the = *(xsMachine **)refcon;

	for (what = kModInstrumentationPixelsDrawn; what <= kModInstrumentationSystemFreeMemory; what++)
		values[what - kModInstrumentationPixelsDrawn] = modInstrumentationGet_(the, what);

	if (values[kModInstrumentationTurns - kModInstrumentationPixelsDrawn])
        values[kModInstrumentationTurns - kModInstrumentationPixelsDrawn] -= 1;     // ignore the turn that generates instrumentation

	fxSampleInstrumentation(the, espInstrumentCount, values);

	modInstrumentationSet(PixelsDrawn, 0);
	modInstrumentationSet(FramesDrawn, 0);
	modInstrumentationSet(PocoDisplayListUsed, 0);
	modInstrumentationSet(PiuCommandListUsed, 0);
	modInstrumentationSet(NetworkBytesRead, 0);
	modInstrumentationSet(NetworkBytesWritten, 0);
	modInstrumentationSet(Turns, 0);
	modInstrumentMachineReset(the);
}

#endif

/*
	messages
*/
int modMessagePostToMachine(xsMachine *the, uint8_t *message, uint16_t messageLength, modMessageDeliver callback, void *refcon)
{
#ifdef mxDebug
	if (0xffff == messageLength)
		messageLength = 0;
#endif

	txZephyrMessage *msg = c_malloc(sizeof(txZephyrMessage) + messageLength);
	if (!msg)
		return -1;
	msg->callback = callback;
	msg->refcon = refcon;
	msg->isStatic = 0;

	if (message && messageLength)
		c_memmove(msg->data, message, messageLength);
	msg->length = messageLength;


	k_fifo_put(&the->messageQueue, msg);

	return 0;
}

int modMessagePostToMachineFromISR(xsMachine *the, modMessageDeliver callback, void *refcon)
{
	txZephyrMessage *msg = fxAllocateStaticMessage();
	if (!msg)
		return -1;
	msg->callback = callback;
	msg->refcon = refcon;
	msg->length = 0;

	k_fifo_put(&the->messageQueue, msg);

	return 0;
}

int modMessageService(xsMachine *the, int maxDelayMS)
{
	k_timeout_t timeout = (maxDelayMS < 0) ? K_FOREVER : K_MSEC(maxDelayMS);
	txZephyrMessage *msg = k_fifo_get(&the->messageQueue, timeout);

	if (!msg)
		return 0;

	if (msg->callback)
		(*msg->callback)(the, msg->refcon, msg->data, msg->length);

	fxFreeZephyrMessage(msg);

	return 1;
}

void modMachineTaskInit(xsMachine *the)
{
        the->task = k_current_get();
}

void modMachineTaskUninit(xsMachine *the)
{
        (void)the;
}


/*
	promises
*/

static void doRunPromiseJobs(void *machine, void *refcon, uint8_t *message, uint16_t messageLength)
{
	fxRunPromiseJobs((txMachine *)machine);
}

void fxQueuePromiseJobs(txMachine* the)
{
	modMessagePostToMachine(the, NULL, 0, doRunPromiseJobs, NULL);
}

/*
	 user installable modules
*/

