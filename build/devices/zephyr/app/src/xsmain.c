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

#include "xs.h"
#include "xsmain.h"
#include "modTimer.h"
#include "modInstrumentation.h"

#include "xsPlatform.h"
#include "xsHost.h"
#include "xsHosts.h"
#include "mc.xs.h"

#include <stddef.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#ifdef mxDebug
extern int fxIsConnected(xsMachine *the);
#endif

#ifndef MODDEF_XS_TEST
        #define MODDEF_XS_TEST 1
#endif

#if !MODDEF_XS_TEST
static
#endif
	xsMachine *gThe;		// the main XS virtual machine running

static void xs_setup_log_creation(const xsCreation *creation)
{
        if (!creation)
                return;
        printk("[zephyr] creation chunk=%ld/%ld heap=%ld/%ld stack=%ld static=%ld\n",
               (long)creation->initialChunkSize,
               (long)creation->incrementalChunkSize,
               (long)creation->initialHeapCount,
               (long)creation->incrementalHeapCount,
               (long)creation->stackCount,
               (long)creation->staticSize);
}

static void xs_setup_prepare_creation(xsCreation *creationOut)
{
        xsCreation *defaults = NULL;
        xsPreparationAndCreation(&defaults);
        if (!creationOut || !defaults)
                return;

        *creationOut = *defaults;

        if (creationOut->initialChunkSize < 16384)
                creationOut->initialChunkSize = 16384;
        if (creationOut->incrementalChunkSize < 4096)
                creationOut->incrementalChunkSize = 4096;
        if (creationOut->initialHeapCount < 8192)
                creationOut->initialHeapCount = 8192;
        if (creationOut->incrementalHeapCount < 1024)
                creationOut->incrementalHeapCount = 1024;

        if (creationOut->staticSize != 0)
                creationOut->staticSize = 0;
}

static void xs_setup_log(const char *stage)
{
        printk("[zephyr] %s\n", stage);
}

void xs_setup(void)
{
        xs_setup_log("xs_setup begin");

#if defined(mxDebug)
        setupDebugger();
        xs_setup_log("setupDebugger complete");
#endif

        while (true) {
                xsCreation creation = {0};
                xs_setup_prepare_creation(&creation);
                xs_setup_log_creation(&creation);

                xs_setup_log("modCloneMachine begin");
                gThe = modCloneMachine(&creation, NULL);
                if (!gThe) {
                        xs_setup_log("modCloneMachine failed");
                        break;
                }
                xs_setup_log("modCloneMachine success");

                xs_setup_log("modRunMachineSetup begin");
                modRunMachineSetup(gThe);
                xs_setup_log("modRunMachineSetup end");

#if MODDEF_XS_TEST
                xsMachine *the = gThe;
                uint32_t turn = 0;
                while (gThe) {
                        if ((turn++ % 100) == 0)
                                xs_setup_log("event loop iteration");
                        modTimersExecute();
                        modMessageService(the, modTimersNext());

                        modInstrumentationAdjust(Turns, +1);
                }
                xs_setup_log("event loop exit");
                xsDeleteMachine(the);
                xs_setup_log("xsDeleteMachine complete");
#else
                while (true) {
                        modTimersExecute();
                        modMessageService(the, modTimersNext());

                        modInstrumentationAdjust(Turns, +1);
                }
#endif
        }

        xs_setup_log("xs_setup end");
}

static void xs_printk_line(const char *msg)
{
        size_t length = c_strlen(msg);
        if (length && ('\n' == msg[length - 1]))
                printk("%s", msg);
        else
                printk("%s\n", msg);
}

void modLog_transmit(const char *msg)
{
        if (!msg)
                return;

#ifdef mxDebug
        if (gThe && fxIsConnected(gThe)) {
                uint8_t c;
                while (0 != (c = c_read8(msg++)))
                        fx_putc(gThe, c);
                fx_putc(gThe, 0);
                return;
        }
#endif

        xs_printk_line(msg);
}

