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

#ifndef __MODGPIO_H__
#define __MODGPIO_H__

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>

#ifndef MODDEF_GPIO_DEFAULT_PORT
#define MODDEF_GPIO_DEFAULT_PORT "GPIO_0"
#endif

enum {
        kModGPIOInput = 0,
        kModGPIOInputPullUp = 1,
        kModGPIOInputPullDown = 2,
        kModGPIOInputPullUpDown = 3,

        kModGPIOWakeRisingEdge  = 1 << 6,
        kModGPIOWakeFallingEdge = 1 << 7,

        kModGPIOOutput = 8,
        kModGPIOOutputOpenDrain = 9
};

typedef struct modGPIOConfigurationRecord modGPIOConfigurationRecord;
typedef struct modGPIOConfigurationRecord *modGPIOConfiguration;

struct modGPIOConfigurationRecord {
        const struct device *device;
        const char *portName;
        gpio_pin_t pin;
        gpio_flags_t flags;
        uint8_t direction;
};

typedef struct modGPIOConfigurationRecord modGPIOConfigurationRecord;
typedef struct modGPIOConfigurationRecord *modGPIOConfiguration;

int modGPIOInit(modGPIOConfiguration config, const char *port, uint8_t pin, uint32_t mode);
void modGPIOUninit(modGPIOConfiguration config);
int modGPIOSetMode(modGPIOConfiguration config, uint32_t mode);

#define kModGPIOReadError (255)
uint8_t modGPIORead(modGPIOConfiguration config);
void modGPIOWrite(modGPIOConfiguration config, uint8_t value);

uint8_t modGPIODidWake(modGPIOConfiguration config, uint8_t pin);

#endif /* __MODGPIO_H__ */
