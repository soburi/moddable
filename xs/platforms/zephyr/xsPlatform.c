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

#include "xsAll.h"
#include "stdio.h"

#include "xsPlatform.h"

#include "mc.defines.h"
#include "xs.h"
#include "xsHosts.h"
#include "xsHost.h"
#include <zephyr/kernel.h>

#ifdef mxDebug
	#include "modPreference.h"
#endif

#define XSDEBUG_NONE	0,0,0,0
#define XSDEBUG_SERIAL	127,0,0,7

#ifndef DEBUG_IP
	#define DEBUG_IP XSDEBUG_SERIAL
#endif

uint8_t gXSBUG[4] = { DEBUG_IP };


#define isSerialIP(ip) ((127 == ip[0]) && (0 == ip[1]) && (0 == ip[2]) && (7 == ip[3]))
#define kSerialConnection ((txSocket)0x87654321)

static void fx_putpi(txMachine *the, char separator, txBoolean trailingcrlf);
static void doRemoteCommand(txMachine *the, uint8_t *cmd, uint32_t cmdLen);
void fxReceiveLoop(void);


int modMessagePostToMachine(xsMachine *the, uint8_t *message, uint16_t messageLength, modMessageDeliver callback, void *refcon);
int modMessagePostToMachineFromISR(xsMachine *the, modMessageDeliver callback, void *refcon);
int modMessageService(xsMachine *the, int maxDelayMS);
void modMachineTaskInit(xsMachine *the);
void modMachineTaskUninit(xsMachine *the);
void modMachineTaskWait(xsMachine *the);
void modMachineTaskWake(xsMachine *the);


void fxCreateMachinePlatform(txMachine* the)
{
        k_fifo_init(&the->messageQueue);
        the->task = k_current_get();
#ifdef mxDebug
        the->inPrintf = 0;
#endif
}

void fxDeleteMachinePlatform(txMachine* the)
{
        txZephyrMessage *msg;
        while ((msg = k_fifo_get(&the->messageQueue, K_NO_WAIT)))
                c_free(msg);
}

void fx_putc(void *refcon, char c)
{
#ifdef mxDebug
    txMachine* the = refcon;

    if (the && the->inPrintf) {
        if (0 == c) {
                        ESP_putc('\r');
                        ESP_putc('\n');
                        the->inPrintf = false;
                        return;
        }
    }
    else {
        if (0 == c)
            return;

        the->inPrintf = true;
        fx_putpi(the, '.', true);
    }
#endif

    ESP_putc(c);
}

void fx_putpi(txMachine *the, char separator, txBoolean trailingcrlf)
{
    static const char *xsbugHeaderStart = "\r\n<?xs";
    static const char *xsbugHeaderEnd = "?>";
    static const char *gHex = "0123456789ABCDEF";
    const char *cp = xsbugHeaderStart;
    while (*cp)
                ESP_putc(*cp++);
        ESP_putc(separator);
        for (int i = 7; i >= 0; i--)
                ESP_putc(gHex[((uintptr_t)the >> (i << 2)) & 0x0F]);
        cp = xsbugHeaderEnd;
        while (*cp)
                ESP_putc(*cp++);
        if (trailingcrlf) {
                ESP_putc('\r');
                ESP_putc('\n');
    }
}

void fxAbort(txMachine* the, int status)
{
        (void)status;
        while (1)
                k_sleep(K_FOREVER);
}

#ifdef mxDebug

static void doDebugCommand(void *machine, void *refcon, uint8_t *message, uint16_t messageLength);

void fxConnect(txMachine* the)
{
}

void fxDisconnect(txMachine* the)
{
}

txBoolean fxIsConnected(txMachine* the)
{
	return false;
}

txBoolean fxIsReadable(txMachine* the)
{
	return 0;
}

#define kXsbugConnectionTimeout 4000	//@@ was 2000 - needs revisiting (pico hosted from ubuntu VM)
void fxReceive(txMachine* the)
{
}

void doDebugCommand(void *machine, void *refcon, uint8_t *message, uint16_t messageLength)
{
}

void fxReceiveLoop(void)
{
}

void fxSend(txMachine* the, txBoolean flags)
{
}

void doRemoteCommand(txMachine *the, uint8_t *cmd, uint32_t cmdLen)
{
}

#endif /* mxDebug */

uint8_t fxInNetworkDebugLoop(txMachine *the)
{
#ifdef mxDebug
	return the->DEBUG_LOOP && the->connection && (kSerialConnection != the->connection);
#else
	return 0;
#endif
}

int modMessagePostToMachine(xsMachine *the, uint8_t *message, uint16_t messageLength, modMessageDeliver callback, void *refcon)
{
        txZephyrMessage *msg = c_malloc(sizeof(txZephyrMessage) + messageLength);
        if (!msg)
                return 1;
        msg->callback = callback;
        msg->refcon = refcon;
        msg->length = messageLength;
        if (messageLength && message)
                c_memmove(msg->data, message, messageLength);
        k_fifo_put(&the->messageQueue, msg);
        return 0;
}

int modMessagePostToMachineFromISR(xsMachine *the, modMessageDeliver callback, void *refcon)
{
        return modMessagePostToMachine(the, NULL, 0, callback, refcon);
}

int modMessageService(xsMachine *the, int maxDelayMS)
{
        k_timeout_t timeout = (maxDelayMS < 0) ? K_FOREVER : K_MSEC(maxDelayMS);
        txZephyrMessage *msg = k_fifo_get(&the->messageQueue, timeout);
        if (!msg)
                return 0;
        if (msg->callback)
                (*msg->callback)(the, msg->refcon, msg->data, msg->length);
        c_free(msg);
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

void modMachineTaskWait(xsMachine *the)
{
        (void)the;
}

void modMachineTaskWake(xsMachine *the)
{
        (void)the;
}
