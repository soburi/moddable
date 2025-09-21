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

#include "modSPI.h"
#include "mc.defines.h"

#include <string.h>

#ifndef MODDEF_SPI_BUFFERSIZE
#define MODDEF_SPI_BUFFERSIZE 512
#endif

static modSPIConfiguration gConfig = NULL;
static uint8_t *gSPITxBuffer = NULL;
static uint16_t gBufferSize = MODDEF_SPI_BUFFERSIZE;

typedef uint16_t (*modSPIBufferLoader)(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);

static uint16_t modSpiLoadBufferAsIs(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);
static uint16_t modSpiLoadBufferSwap16(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);
static uint16_t modSpiLoadBufferRGB565LEtoRGB444(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);
static uint16_t modSpiLoadBufferRGB332To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);
static uint16_t modSpiLoadBufferGray256To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);
static uint16_t modSpiLoadBufferGray16To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut);

static void ensureBuffer(void)
{
        if (!gSPITxBuffer)
                gSPITxBuffer = c_malloc(gBufferSize);
}

static void configureOperation(modSPIConfiguration config)
{
        uint16_t operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);

        if (config->mode & 0x02)
                operation |= SPI_MODE_CPOL;
        if (config->mode & 0x01)
                operation |= SPI_MODE_CPHA;

        config->config.operation = operation;
        config->config.frequency = config->hz;
        config->config.slave = 0;
        config->config.cs = NULL;
}

void modSPIInit(modSPIConfiguration config)
{
        if (!config->bus_name)
                config->bus_name = MODDEF_SPI_DEFAULT_PORT;

        config->bus = device_get_binding(config->bus_name);
        if (!config->bus) {
                modLog("SPI bus not found");
                return;
        }

        configureOperation(config);
        ensureBuffer();

        if (config->doChipSelect)
                config->doChipSelect(0, config);
}

void modSPIUninit(modSPIConfiguration config)
{
        if (config == gConfig)
                modSPIActivateConfiguration(NULL);
}

void modSPIActivateConfiguration(modSPIConfiguration config)
{
        modSPIFlush();

        if (config == gConfig)
                return;

        if (gConfig && gConfig->doChipSelect)
                gConfig->doChipSelect(0, gConfig);

        gConfig = config;

        if (gConfig) {
                configureOperation(gConfig);
                if (gConfig->doChipSelect)
                        gConfig->doChipSelect(1, gConfig);
        }
}

static void modSPITxCommon(modSPIConfiguration config, uint8_t *data, uint16_t count, modSPIBufferLoader loader)
{
        uint16_t bitsOut;
        modSPIActivateConfiguration(config);
        ensureBuffer();

        while (count > 0) {
                uint16_t consumed = loader(data, count, &bitsOut);
                uint16_t bytesOut = (bitsOut + 7) >> 3;
                struct spi_buf buf = {
                        .buf = gSPITxBuffer,
                        .len = bytesOut
                };
                struct spi_buf_set tx = {
                        .buffers = &buf,
                        .count = 1
                };
                int err = spi_write(config->bus, &config->config, &tx);
                if (0 != err) {
                        modLog("SPI write failed");
                        modLogInt(err);
                        break;
                }
                data += consumed;
                count -= consumed;
        }
}

void modSPITx(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferAsIs);
}

void modSPITxSwap16(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferSwap16);
}

void modSPITxRGB565LEtoRGB444(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferRGB565LEtoRGB444);
}

void modSPITxRGB332To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferRGB332To16BE);
}

void modSPITxGray256To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferGray256To16BE);
}

void modSPITxGray16To16BE(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        modSPITxCommon(config, data, count, modSpiLoadBufferGray16To16BE);
}

void modSPITxRx(modSPIConfiguration config, uint8_t *data, uint16_t count)
{
        struct spi_buf tx_buf = {
                .buf = data,
                .len = count
        };
        struct spi_buf rx_buf = {
                .buf = gSPITxBuffer,
                .len = count
        };
        struct spi_buf_set tx = { &tx_buf, 1 };
        struct spi_buf_set rx = { &rx_buf, 1 };

        modSPIActivateConfiguration(config);
        ensureBuffer();

        int err = spi_transceive(config->bus, &config->config, &tx, &rx);
        if (0 != err) {
                modLog("SPI transceive failed");
                modLogInt(err);
                return;
        }

        c_memmove(data, gSPITxBuffer, count);
}

void modSPIFlush(void)
{
}

static uint16_t modSpiLoadBufferAsIs(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        if (bytes > gBufferSize)
                bytes = gBufferSize;
        c_memmove(gSPITxBuffer, data, bytes);
        *bitsOut = bytes << 3;
        return bytes;
}

static uint16_t modSpiLoadBufferSwap16(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        if (bytes > gBufferSize)
                bytes = gBufferSize;
        bytes &= ~1;     // keep even count
        for (uint16_t i = 0; i < bytes; i += 2) {
                gSPITxBuffer[i] = data[i + 1];
                gSPITxBuffer[i + 1] = data[i];
        }
        *bitsOut = bytes << 3;
        return bytes;
}

static uint16_t modSpiLoadBufferRGB565LEtoRGB444(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        uint16_t *from = (uint16_t *)data;
        uint8_t *to = gSPITxBuffer;
        uint16_t remain;

        if (bytes > gBufferSize)
                bytes = gBufferSize;

        *bitsOut = (bytes >> 1) * 12;
        remain = bytes >> 2;
        while (remain--) {
                uint8_t r, g, b;
                uint16_t rgb565 = *from++;
                r = rgb565 >> 12;
                g = (rgb565 >> 7) & 0x0F;
                b = (rgb565 >> 1) & 0x0F;
                *to++ = (r << 4) | g;

                rgb565 = *from++;
                r = rgb565 >> 12;
                *to++ = (b << 4) | r;
                g = (rgb565 >> 7) & 0x0F;
                b = (rgb565 >> 1) & 0x0F;
                *to++ = (g << 4) | b;
        }

        if (bytes & 2) {
                uint8_t r, g, b;
                uint16_t rgb565 = *from;
                r = rgb565 >> 12;
                g = (rgb565 >> 7) & 0x0F;
                b = (rgb565 >> 1) & 0x0F;
                *to++ = (r << 4) | g;
                *to = b << 4;
        }

        return bytes;
}

static uint16_t modSpiLoadBufferRGB332To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        uint8_t *from = data;
        uint32_t *to = (uint32_t *)gSPITxBuffer;
        uint16_t remain;

        if (bytes > (gBufferSize >> 1))
                bytes = gBufferSize >> 1;

        *bitsOut = bytes << 4;

        remain = bytes;
        while (remain >= 2) {
                uint8_t rgb332;
                uint16_t pixela, pixelb;

                rgb332 = *from++;
                uint8_t r = rgb332 >> 5;
                uint8_t g = (rgb332 >> 2) & 0x07;
                uint8_t b = rgb332 & 0x03;
                r = (r << 2) | (r >> 2);
                g |= g << 3;
                b = (b << 3) | (b << 1) | (b >> 1);
                pixela = (r << 11) | (g << 5) | b;
                pixela = (pixela >> 8) | (pixela << 8);

                rgb332 = *from++;
                r = rgb332 >> 5;
                g = (rgb332 >> 2) & 0x07;
                b = rgb332 & 0x03;
                r = (r << 2) | (r >> 2);
                g |= g << 3;
                b = (b << 3) | (b << 1) | (b >> 1);
                pixelb = (r << 11) | (g << 5) | b;
                pixelb = (pixelb >> 8) | (pixelb << 8);

                *to++ = (pixelb << 16) | pixela;
                remain -= 2;
        }

        if (remain) {
                uint8_t rgb332 = *from++;
                uint8_t r = rgb332 >> 5;
                uint8_t g = (rgb332 >> 2) & 0x07;
                uint8_t b = rgb332 & 0x03;
                r = (r << 2) | (r >> 2);
                g |= g << 3;
                b = (b << 3) | (b << 1) | (b >> 1);
                uint16_t pixela = (r << 11) | (g << 5) | b;
                pixela = (pixela >> 8) | (pixela << 8);
                *to++ = ((uint32_t)0 << 16) | pixela;
        }

        return bytes;
}

static uint16_t modSpiLoadBufferGray256To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        uint8_t *from = data;
        uint32_t *to = (uint32_t *)gSPITxBuffer;
        uint16_t remain;

        if (bytes > (gBufferSize >> 1))
                bytes = gBufferSize >> 1;

        *bitsOut = bytes << 4;

        remain = bytes;
        while (remain >= 2) {
                uint8_t gray = *from++;
                uint16_t pixela = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
                pixela = (pixela >> 8) | (pixela << 8);

                gray = *from++;
                uint16_t pixelb = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
                pixelb = (pixelb >> 8) | (pixelb << 8);

                *to++ = (pixelb << 16) | pixela;
                remain -= 2;
        }

        if (remain) {
                uint8_t gray = *from++;
                uint16_t pixela = ((gray >> 3) << 11) | ((gray >> 2) << 5) | (gray >> 3);
                pixela = (pixela >> 8) | (pixela << 8);
                *to++ = ((uint32_t)0 << 16) | pixela;
        }

        return bytes;
}

static uint16_t modSpiLoadBufferGray16To16BE(uint8_t *data, uint16_t bytes, uint16_t *bitsOut)
{
        uint8_t *from = data;
        uint32_t *to = (uint32_t *)gSPITxBuffer;

        if (bytes > (gBufferSize >> 2))
                bytes = gBufferSize >> 2;

        *bitsOut = bytes << 5;

        for (uint16_t i = 0; i < bytes; i++) {
            uint8_t twoPixels = *from++;
            uint8_t gray = (twoPixels >> 2) & 0x3C;
            gray |= gray >> 4;
            uint16_t pixela = (gray >> 1);
            pixela |= (pixela << 11) | (gray << 5);
            pixela = (pixela >> 8) | (pixela << 8);

            gray = (twoPixels << 2) & 0x3C;
            gray |= gray >> 4;
            uint16_t pixelb = (gray >> 1);
            pixelb |= (pixelb << 11) | (gray << 5);
            pixelb = (pixelb >> 8) | (pixelb << 8);

            *to++ = (pixelb << 16) | pixela;
        }

        return bytes;
}
