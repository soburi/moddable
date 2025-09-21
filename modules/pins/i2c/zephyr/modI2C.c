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

#include "modI2C.h"
#include "mc.defines.h"

#ifndef MODDEF_I2C_TIMEOUT_MS
#define MODDEF_I2C_TIMEOUT_MS 100
#endif

static uint32_t modI2CClockToSpeed(uint32_t hz)
{
        if (hz >= 3400000)
                return I2C_SPEED_HIGH;
        if (hz >= 1000000)
                return I2C_SPEED_FAST_PLUS;
        if (hz >= 400000)
                return I2C_SPEED_FAST;
        return I2C_SPEED_STANDARD;
}

void modI2CInit(modI2CConfiguration config)
{
        config->bus_name = config->bus_name ? config->bus_name : MODDEF_I2C_BUS;
        config->bus = device_get_binding(config->bus_name);
        if (!config->bus) {
                modLog("I2C bus not found");
                return;
        }

        uint32_t speed = modI2CClockToSpeed(config->hz);
        int err = i2c_configure(config->bus, I2C_MODE_MASTER | I2C_SPEED_SET(speed));
        if (0 != err)
                modLogInt(err);

        if (0 == config->timeout)
                config->timeout = MODDEF_I2C_TIMEOUT_MS;
}

void modI2CUninit(modI2CConfiguration config)
{
        (void)config;
}

uint8_t modI2CRead(modI2CConfiguration config, uint8_t *buffer, uint16_t length, uint8_t sendStop)
{
        if (!config->bus)
                return 1;

        struct i2c_msg msg = {
                .buf = buffer,
                .len = length,
                .flags = I2C_MSG_READ | (sendStop ? I2C_MSG_STOP : 0)
        };
        int err = i2c_transfer(config->bus, &msg, 1, config->address);
        if (0 != err) {
                modLog("I2C read failed");
                modLogInt(err);
                return 1;
        }
        return 0;
}

uint8_t modI2CWrite(modI2CConfiguration config, const uint8_t *buffer, uint16_t length, uint8_t sendStop)
{
        if (!config->bus)
                return 1;

        struct i2c_msg msg = {
                .buf = (uint8_t *)buffer,
                .len = length,
                .flags = I2C_MSG_WRITE | (sendStop ? I2C_MSG_STOP : 0)
        };
        int err = i2c_transfer(config->bus, &msg, 1, config->address);
        if (0 != err) {
                modLog("I2C write failed");
                modLogInt(err);
                return 1;
        }
        return 0;
}
