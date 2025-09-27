/*
 * Simulation servo implementation without XS dependencies.
 */

#include "servo_platform.h"

#include "xsHost.h"

#include <stdint.h>

struct modServoPlatformRecord {
        int pin;
        uint8_t running;
};

typedef struct modServoPlatformRecord modServoPlatformRecord;

modServoStatus modServoPlatformCreate(const modServoConfiguration *config, modServoPlatform *outServo)
{
        modServoPlatformRecord *servo;

        if (!config || !outServo)
                return kModServoStatusInvalidArgument;

        servo = c_calloc(1, sizeof(modServoPlatformRecord));
        if (!servo)
                return kModServoStatusNoMemory;

        servo->pin = config->pin;
        servo->running = true;

        *outServo = servo;
        return kModServoStatusOK;
}

void modServoPlatformClose(modServoPlatform platform)
{
        modServoPlatformRecord *servo = platform;
        if (!servo || !servo->running)
                return;

        servo->running = false;
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

        if (!servo || !servo->running)
                return kModServoStatusNotOpen;

        (void)degrees;
        return kModServoStatusOK;
}

modServoStatus modServoPlatformWriteMicros(modServoPlatform platform, double microseconds)
{
        modServoPlatformRecord *servo = platform;

        if (!servo || !servo->running)
                return kModServoStatusNotOpen;

        (void)microseconds;
        return kModServoStatusOK;
}
