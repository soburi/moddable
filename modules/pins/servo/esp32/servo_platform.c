/*
 * ESP32 servo hardware control separated from XS bindings.
 */

#include "servo_platform.h"

#include "xsHost.h"

#include "driver/ledc.h"

#include <stdint.h>

struct modServoPlatformRecord {
        int minTicks;
        int maxTicks;
        uint8_t running;
        uint8_t channel;
};

typedef struct modServoPlatformRecord modServoPlatformRecord;

static uint8_t gChannelsUsed = 0;

static inline double servoResolutionScale(void)
{
#if SOC_LEDC_TIMER_BIT_WIDE_NUM > 14
        return 32767.0;
#else
        return 16383.0;
#endif
}

static inline ledc_mode_t servoSpeedMode(void)
{
#if SOC_LEDC_SUPPORT_HS_MODE
        return LEDC_HIGH_SPEED_MODE;
#else
        return LEDC_LOW_SPEED_MODE;
#endif
}

modServoStatus modServoPlatformCreate(const modServoConfiguration *config, modServoPlatform *outServo)
{
        modServoPlatformRecord *servo;
        ledc_timer_config_t timer = {
#if SOC_LEDC_TIMER_BIT_WIDE_NUM > 14
                .duty_resolution = LEDC_TIMER_15_BIT,
#else
                .duty_resolution = LEDC_TIMER_14_BIT,
#endif
                .freq_hz = 50,
                .speed_mode = servoSpeedMode(),
                .timer_num = LEDC_TIMER_0
        };
        ledc_channel_config_t channelConfig = {
                .channel = LEDC_CHANNEL_0,
                .duty = 0,
                .gpio_num = config ? config->pin : 0,
                .speed_mode = servoSpeedMode(),
                .timer_sel = LEDC_TIMER_0
        };
        double scale = servoResolutionScale();
        uint8_t channel = 0;

        if (!config || !outServo)
                return kModServoStatusInvalidArgument;

        servo = c_calloc(1, sizeof(modServoPlatformRecord));
        if (!servo)
                return kModServoStatusNoMemory;

#if SOC_LEDC_TIMER_BIT_WIDE_NUM > 14
        servo->minTicks = 890;
        servo->maxTicks = 4000;
#else
        servo->minTicks = 445;
        servo->maxTicks = 2000;
#endif

        if (config->hasMin)
                servo->minTicks = (int)((config->min / 20000.0) * scale);
        if (config->hasMax)
                servo->maxTicks = (int)((config->max / 20000.0) * scale);

        if (config->hasChannel) {
                if ((config->channel < 0) || (config->channel >= LEDC_CHANNEL_MAX)) {
                        c_free(servo);
                        return kModServoStatusInvalidArgument;
                }
                channel = (uint8_t)config->channel;
                if (gChannelsUsed & (1 << channel)) {
                        c_free(servo);
                        return kModServoStatusChannelUnavailable;
                }
        } else {
                uint8_t candidate = 0;
                while ((candidate < LEDC_CHANNEL_MAX) && (gChannelsUsed & (1 << candidate)))
                        candidate++;
                if (candidate == LEDC_CHANNEL_MAX) {
                        c_free(servo);
                        return kModServoStatusChannelUnavailable;
                }
                channel = candidate;
        }

        channelConfig.channel = channel;
        gChannelsUsed |= (1 << channel);

        if (ledc_timer_config(&timer) != ESP_OK) {
                gChannelsUsed &= ~(1 << channel);
                c_free(servo);
                return kModServoStatusHardwareError;
        }

        channelConfig.gpio_num = config->pin;

        if (ledc_channel_config(&channelConfig) != ESP_OK) {
                gChannelsUsed &= ~(1 << channel);
                c_free(servo);
                return kModServoStatusHardwareError;
        }

        servo->channel = channel;
        servo->running = true;

        *outServo = servo;
        return kModServoStatusOK;
}

void modServoPlatformClose(modServoPlatform platform)
{
        modServoPlatformRecord *servo = platform;
        if (!servo || !servo->running)
                return;

        ledc_stop(servoSpeedMode(), servo->channel, 0);
        servo->running = false;
        gChannelsUsed &= ~(1 << servo->channel);
}

void modServoPlatformDelete(modServoPlatform platform)
{
        modServoPlatformRecord *servo = platform;
        if (!servo)
                return;

        modServoPlatformClose(servo);
        c_free(servo);
}

modServoStatus modServoPlatformWriteDegrees(modServoPlatform platform, double degrees)
{
        modServoPlatformRecord *servo = platform;
        int duty;

        if (!servo || !servo->running)
                return kModServoStatusNotOpen;

        duty = (int)(((double)(servo->maxTicks - servo->minTicks) * degrees) / 180.0) + servo->minTicks;

        if (ledc_set_duty(servoSpeedMode(), servo->channel, duty) != ESP_OK)
                return kModServoStatusHardwareError;
        if (ledc_update_duty(servoSpeedMode(), servo->channel) != ESP_OK)
                return kModServoStatusHardwareError;

        return kModServoStatusOK;
}

modServoStatus modServoPlatformWriteMicros(modServoPlatform platform, double microseconds)
{
        modServoPlatformRecord *servo = platform;
        int duty;

        if (!servo || !servo->running)
                return kModServoStatusNotOpen;

        duty = (int)((microseconds / 20000.0) * servoResolutionScale());

        if (ledc_set_duty(servoSpeedMode(), servo->channel, duty) != ESP_OK)
                return kModServoStatusHardwareError;
        if (ledc_update_duty(servoSpeedMode(), servo->channel) != ESP_OK)
                return kModServoStatusHardwareError;

        return kModServoStatusOK;
}
