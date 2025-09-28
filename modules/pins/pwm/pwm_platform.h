#ifndef MOD_PWM_PLATFORM_H
#define MOD_PWM_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct modPWMPlatformRecord *modPWMPlatform;

typedef struct {
        int pin;
        int port;
        bool hasPort;
} modPWMSetup;

typedef enum {
        kModPWMStatusOK = 0,
        kModPWMStatusInvalidArgument,
        kModPWMStatusChannelUnavailable,
        kModPWMStatusNoMemory,
        kModPWMStatusHardwareError,
} modPWMStatus;

modPWMStatus modPWMPlatformCreate(const modPWMSetup *setup, modPWMPlatform *platform);
void modPWMPlatformDelete(modPWMPlatform platform);
modPWMStatus modPWMPlatformWrite(modPWMPlatform platform, int value);

#ifdef __cplusplus
}
#endif

#endif
