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
#include "xsHost.h"

#include "modGPIO.h"

static const struct device *modGPIOResolvePort(const char *portName)
{
        const struct device *dev = device_get_binding(portName);
        if (!dev) {
                modLog("GPIO port not found");
        }
        return dev;
}

static int modGPIOConfigure(modGPIOConfiguration config, gpio_flags_t flags)
{
        int err = gpio_pin_configure(config->device, config->pin, flags);
        if (0 != err) {
                modLog("GPIO configure failed");
                modLogInt(err);
                return err;
        }
        config->flags = flags;
        return 0;
}

int modGPIOInit(modGPIOConfiguration config, const char *port, uint8_t pin, uint32_t mode)
{
        config->pin = pin;
        config->direction = 0xFF;
        config->portName = port ? port : MODDEF_GPIO_DEFAULT_PORT;
        config->device = modGPIOResolvePort(config->portName);
        if (!config->device)
                return -1;

        return modGPIOSetMode(config, mode);
}

void modGPIOUninit(modGPIOConfiguration config)
{
        if (!config->device)
                return;
#ifdef GPIO_DISCONNECTED
        gpio_pin_configure(config->device, config->pin, GPIO_DISCONNECTED);
#else
        gpio_pin_configure(config->device, config->pin, GPIO_INPUT);
#endif
        config->device = NULL;
        config->direction = 0xFF;
}

int modGPIOSetMode(modGPIOConfiguration config, uint32_t mode)
{
        gpio_flags_t flags = 0;
        uint32_t wakeMask = mode & (kModGPIOWakeRisingEdge | kModGPIOWakeFallingEdge);
        if (wakeMask)
                mode &= ~(kModGPIOWakeRisingEdge | kModGPIOWakeFallingEdge);

        switch (mode) {
                case kModGPIOInput:
                        flags = GPIO_INPUT;
                        config->direction = 0;
                        break;
                case kModGPIOInputPullUp:
                        flags = GPIO_INPUT | GPIO_PULL_UP;
                        config->direction = 0;
                        break;
                case kModGPIOInputPullDown:
                        flags = GPIO_INPUT | GPIO_PULL_DOWN;
                        config->direction = 0;
                        break;
                case kModGPIOInputPullUpDown:
                        flags = GPIO_INPUT | GPIO_PULL_UP | GPIO_PULL_DOWN;
                        config->direction = 0;
                        break;
                case kModGPIOOutput:
                        flags = GPIO_OUTPUT;
                        config->direction = 1;
                        break;
                case kModGPIOOutputOpenDrain:
                        flags = GPIO_OUTPUT | GPIO_OPEN_DRAIN;
                        config->direction = 1;
                        break;
                default:
                        return -1;
        }

        int err = modGPIOConfigure(config, flags);
        if (err)
                return err;

        if (wakeMask)
                modLog("GPIO wake edges not supported on Zephyr yet");

        return 0;
}

uint8_t modGPIORead(modGPIOConfiguration config)
{
        if (!config->device)
                return kModGPIOReadError;

        int value = gpio_pin_get(config->device, config->pin);
        if (value < 0)
                return kModGPIOReadError;
        return (uint8_t)value;
}

void modGPIOWrite(modGPIOConfiguration config, uint8_t value)
{
        if (!config->device)
                return;
        gpio_pin_set(config->device, config->pin, value ? 1 : 0);
}

uint8_t modGPIODidWake(modGPIOConfiguration config, uint8_t pin)
{
        (void)config;
        (void)pin;
        return 0;
}
