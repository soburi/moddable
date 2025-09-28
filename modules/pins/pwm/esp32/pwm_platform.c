/*
 * ESP32 PWM hardware abstraction used by the shared XS binding.
 */

#include "pwm_platform.h"

#include "mc.defines.h"
#include "xsPlatform.h"

#include "driver/ledc.h"

#ifndef MODDEF_PWM_LEDC_CHANNEL
        #define MODDEF_PWM_LEDC_CHANNEL_MAP 0xF0
#endif
#ifndef MODDEF_PWM_LEDC_TIMER
        #define MODDEF_PWM_LEDC_TIMER LEDC_TIMER_0
#endif

#if SOC_LEDC_SUPPORT_HS_MODE
        #define ESP_SPEED_MODE LEDC_HIGH_SPEED_MODE
#else
        #define ESP_SPEED_MODE LEDC_LOW_SPEED_MODE
#endif

#ifndef MODDEF_PWM_LEDC_FREQUENCY
        #define MODDEF_PWM_LEDC_FREQUENCY 1024
#endif

#ifndef MODDEF_PWM_LEDC_OFFVALUE
        #define MODDEF_PWM_LEDC_OFFVALUE 0
#endif

typedef struct modPWMPlatformRecord {
        uint32_t gpio;
        uint8_t ledc;
} modPWMPlatformRecord;

static const ledc_timer_config_t gTimer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = MODDEF_PWM_LEDC_FREQUENCY,
        .speed_mode = ESP_SPEED_MODE,
        .timer_num = MODDEF_PWM_LEDC_TIMER,
        .clk_cfg = LEDC_AUTO_CLK
};

static uint8_t gLEDC = MODDEF_PWM_LEDC_CHANNEL_MAP;
static uint8_t gTimerConfigured = 0;

static inline uint32_t modPWMClampDuty(int value)
{
        if (value < 0 || value > 1023)
                return UINT32_MAX;

        return (value == 1023) ? 1024 : (uint32_t)value;
}

modPWMStatus modPWMPlatformCreate(const modPWMSetup *setup, modPWMPlatform *platform)
{
        ledc_channel_config_t ledcConfig = {0};
        modPWMPlatform pwm;
        int ledc;

        if (!setup || !platform)
                return kModPWMStatusInvalidArgument;

        if (setup->hasPort)
                return kModPWMStatusInvalidArgument;

        if (!gTimerConfigured) {
                if (ESP_OK != ledc_timer_config(&gTimer))
                        return kModPWMStatusHardwareError;
                gTimerConfigured = 1;
        }

        for (ledc = 0; ledc < 8; ledc++) {
                if (gLEDC & (1 << ledc))
                        break;
        }

        if (8 == ledc)
                return kModPWMStatusChannelUnavailable;

        ledcConfig.channel = ledc;
        ledcConfig.duty = MODDEF_PWM_LEDC_OFFVALUE ? 1024 : 0;
        ledcConfig.gpio_num = setup->pin;
        ledcConfig.speed_mode = ESP_SPEED_MODE;
        ledcConfig.hpoint = 0;
        ledcConfig.timer_sel = MODDEF_PWM_LEDC_TIMER;
        ledcConfig.intr_type = LEDC_INTR_DISABLE;

        if (ESP_OK != ledc_channel_config(&ledcConfig))
                return kModPWMStatusHardwareError;

        pwm = c_malloc(sizeof(modPWMPlatformRecord));
        if (!pwm)
                return kModPWMStatusNoMemory;

        pwm->gpio = setup->pin;
        pwm->ledc = (uint8_t)ledc;

        gLEDC &= (uint8_t)~(1 << ledc);

        *platform = pwm;
        return kModPWMStatusOK;
}

void modPWMPlatformDelete(modPWMPlatform platform)
{
        modPWMPlatform pwm = platform;
        if (!pwm)
                return;

        ledc_stop(ESP_SPEED_MODE, pwm->ledc, MODDEF_PWM_LEDC_OFFVALUE);
        gLEDC |= (uint8_t)(1 << pwm->ledc);

        c_free(pwm);
}

modPWMStatus modPWMPlatformWrite(modPWMPlatform platform, int value)
{
        modPWMPlatform pwm = platform;
        uint32_t duty;

        if (!pwm)
                return kModPWMStatusHardwareError;

        duty = modPWMClampDuty(value);
        if (UINT32_MAX == duty)
                return kModPWMStatusInvalidArgument;

        if (ESP_OK != ledc_set_duty(ESP_SPEED_MODE, pwm->ledc, duty))
                return kModPWMStatusHardwareError;

        if (ESP_OK != ledc_update_duty(ESP_SPEED_MODE, pwm->ledc))
                return kModPWMStatusHardwareError;

        return kModPWMStatusOK;
}
