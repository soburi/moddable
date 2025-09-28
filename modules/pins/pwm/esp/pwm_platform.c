/*
 * ESP8266 PWM hardware abstraction used by the shared XS binding.
 */

#include "pwm_platform.h"

#include "xsPlatform.h"

#include "Arduino.h"

typedef struct modPWMPlatformRecord {
        int gpio;
} modPWMPlatformRecord;

modPWMStatus modPWMPlatformCreate(const modPWMSetup *setup, modPWMPlatform *platform)
{
        modPWMPlatform pwm;

        if (!setup || !platform)
                return kModPWMStatusInvalidArgument;

        if (setup->hasPort)
                return kModPWMStatusInvalidArgument;

        pwm = c_malloc(sizeof(modPWMPlatformRecord));
        if (!pwm)
                return kModPWMStatusNoMemory;

        pwm->gpio = setup->pin;
        analogWrite(pwm->gpio, 0);

        *platform = pwm;
        return kModPWMStatusOK;
}

void modPWMPlatformDelete(modPWMPlatform platform)
{
        modPWMPlatform pwm = platform;
        if (!pwm)
                return;

        analogWrite(pwm->gpio, 0);
        c_free(pwm);
}

modPWMStatus modPWMPlatformWrite(modPWMPlatform platform, int value)
{
        modPWMPlatform pwm = platform;

        if (!pwm)
                return kModPWMStatusHardwareError;

        analogWrite(pwm->gpio, value);
        return kModPWMStatusOK;
}
