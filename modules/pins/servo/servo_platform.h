#ifndef MOD_SERVO_PLATFORM_H
#define MOD_SERVO_PLATFORM_H

#include <stdbool.h>

typedef struct modServoPlatformRecord *modServoPlatform;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
        int pin;
        bool hasChannel;
        int channel;
        bool hasMin;
        double min;
        bool hasMax;
        double max;
} modServoConfiguration;

typedef enum {
        kModServoStatusOK = 0,
        kModServoStatusNoMemory,
        kModServoStatusChannelUnavailable,
        kModServoStatusHardwareError,
        kModServoStatusInvalidArgument,
        kModServoStatusNotOpen
} modServoStatus;

modServoStatus modServoPlatformCreate(const modServoConfiguration *config, modServoPlatform *outServo);
void modServoPlatformClose(modServoPlatform servo);
void modServoPlatformDelete(modServoPlatform servo);
modServoStatus modServoPlatformWriteDegrees(modServoPlatform servo, double degrees);
modServoStatus modServoPlatformWriteMicros(modServoPlatform servo, double microseconds);

#ifdef __cplusplus
}
#endif

#endif /* MOD_SERVO_PLATFORM_H */
