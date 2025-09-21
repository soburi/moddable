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

#ifndef __XSZEPHYR_PLATFORM__
#define __XSZEPHYR_PLATFORM__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <zephyr/kernel.h>

#define mxLittleEndian 1
#define mxBigEndian 0

#define mxUseGCCAtomics 1
#define mxUseDefaultBuildKeys 1
#define mxUseDefaultChunkAllocation 1
#define mxUseDefaultSlotAllocation 1
#define mxUseDefaultFindModule 1
#define mxUseDefaultLoadModule 1
#define mxUseDefaultParseScript 1
#define mxUseDefaultSharedChunks 1

typedef struct txZephyrMessage txZephyrMessage;

struct txZephyrMessage {
        void *fifo_reserved; /* required by k_fifo */
        modMessageDeliver callback;
        void *refcon;
        uint16_t length;
        uint8_t data[1];
};

#ifdef mxDebug
        #define mxMachineDebug \
                uint8_t inPrintf;
#else
        #define mxMachineDebug
#endif

#ifdef mxInstrument
        #define mxMachineInstrument \
                void *instrumentationTimer; \
                void *instrumentationCallback;
#else
        #define mxMachineInstrument
#endif

#define mxMachinePlatform \
        struct k_fifo messageQueue; \
        void *task; \
        mxMachineDebug \
        mxMachineInstrument

#endif /* __XSZEPHYR_PLATFORM__ */
