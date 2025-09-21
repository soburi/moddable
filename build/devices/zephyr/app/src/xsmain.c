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

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "xs.h"
#include "xsmain.h"
#include "xsHost.h"
#include "xsHosts.h"
#include "modTimer.h"
#include "modInstrumentation.h"

#ifndef MODDEF_XS_TEST
#define MODDEF_XS_TEST 1
#endif

static xsMachine *gThe = NULL;

#ifdef mxDebug
extern void setupDebugger(void);
#endif

void xs_setup(void)
{
#ifdef mxDebug
        setupDebugger();
#endif

        while (true) {
                gThe = modCloneMachine(NULL, NULL);
                modRunMachineSetup(gThe);

#if MODDEF_XS_TEST
                xsMachine *the = gThe;
                while (gThe) {
                        modTimersExecute();
                        modMessageService(the, modTimersNext());
                        modInstrumentationAdjust(Turns, +1);
                }
                xsDeleteMachine(the);
#else
                while (true) {
                        modTimersExecute();
                        modMessageService(gThe, modTimersNext());
                        modInstrumentationAdjust(Turns, +1);
                }
#endif
        }
}

void modLog_transmit(const char *msg)
{
        uint8_t c;

#ifdef mxDebug
        if (gThe) {
                while (0 != (c = c_read8(msg++)))
                        fx_putc(gThe, c);
                fx_putc(gThe, 0);
                return;
        }
#endif

        while (0 != (c = c_read8(msg++)))
                printk("%c", c);
        printk("\r\n");
}
