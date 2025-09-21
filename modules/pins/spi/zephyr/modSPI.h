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

#ifndef __MODSPI_H__
#define __MODSPI_H__

#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include "xsMachine.h"

#ifndef MODDEF_SPI_DEFAULT_PORT
#define MODDEF_SPI_DEFAULT_PORT "SPI_0"
#endif

typedef struct modSPIConfigurationRecord modSPIConfigurationRecord;
typedef struct modSPIConfigurationRecord *modSPIConfiguration;

typedef void (*modSPIChipSelectCallback)(uint8_t status, modSPIConfiguration config);

struct modSPIConfigurationRecord {
        const struct device *bus;
        const char *bus_name;
        const char *cs_port;
        struct spi_config config;
        modSPIChipSelectCallback doChipSelect;
        xsMachine *the;
        uint32_t hz;
        uint8_t mode;
        uint8_t sync;
        uint8_t cs_pin;
        uint8_t clock_pin;
        uint8_t mosi_pin;
        uint8_t miso_pin;
};

#define modSPIConfig(config, HZ, SPI_PORT, CS_PORT, CS_PIN, DOCHIPSELECT) \
        config.hz = HZ; \
        config.bus_name = SPI_PORT; \
        config.cs_port = CS_PORT; \
        config.cs_pin = (uint8_t)((CS_PIN) < 0 ? 0xFF : (CS_PIN)); \
        config.doChipSelect = DOCHIPSELECT; \
        config.mode = 0; \
        config.sync = 1; \
        config.clock_pin = 0xFF; \
        config.mosi_pin = 0xFF; \
        config.miso_pin = 0xFF;

void modSPIInit(modSPIConfiguration config);
void modSPIUninit(modSPIConfiguration config);
void modSPITxRx(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITx(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITxSwap16(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITxRGB565LEtoRGB444(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITxRGB332To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITxGray256To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPITxGray16To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count);
void modSPIFlush(void);
void modSPIActivateConfiguration(modSPIConfiguration config);

#define modSPISetSync(config, _sync) (config)->sync = (_sync)

#endif /* __MODSPI_H__ */
