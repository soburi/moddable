/*
 * Servo XS bindings delegating hardware operations to platform implementations.
 */

#include "xsmc.h"
#include "mc.xs.h"
#include "xsHost.h"

#include "servo_platform.h"

typedef struct {
        modServoPlatform platform;
} xsServoRecord, *xsServo;

static void xsServoThrowStatus(xsMachine *the, modServoStatus status)
{
        switch (status) {
        case kModServoStatusOK:
                break;
        case kModServoStatusNoMemory:
                xsUnknownError("no memory");
                break;
        case kModServoStatusChannelUnavailable:
                xsRangeError("all servo channels are already taken");
                break;
        case kModServoStatusInvalidArgument:
                xsRangeError("invalid servo configuration");
                break;
        case kModServoStatusNotOpen:
                xsUnknownError("closed");
                break;
        case kModServoStatusHardwareError:
        default:
                xsUnknownError("servo hardware error");
                break;
        }
}

void xs_servo_destructor(void *data)
{
        xsServo servo = data;
        if (!servo)
                return;

        if (servo->platform) {
                modServoPlatformDelete(servo->platform);
                servo->platform = NULL;
        }

        c_free(servo);
}

void xs_servo(xsMachine *the)
{
        xsServo servo = c_calloc(1, sizeof(xsServoRecord));
        modServoConfiguration config = {0};
        modServoStatus status;

        if (!servo)
                xsUnknownError("no memory");

        xsmcVars(1);
        xsmcGet(xsVar(0), xsArg(0), xsID_pin);
        config.pin = xsmcToInteger(xsVar(0));

        if (xsmcHas(xsArg(0), xsID_channel)) {
                xsmcGet(xsVar(0), xsArg(0), xsID_channel);
                config.hasChannel = true;
                config.channel = xsmcToInteger(xsVar(0));
        }

        if (xsmcHas(xsArg(0), xsID_min)) {
                xsmcGet(xsVar(0), xsArg(0), xsID_min);
                config.hasMin = true;
                config.min = xsmcToNumber(xsVar(0));
        }

        if (xsmcHas(xsArg(0), xsID_max)) {
                xsmcGet(xsVar(0), xsArg(0), xsID_max);
                config.hasMax = true;
                config.max = xsmcToNumber(xsVar(0));
        }

        status = modServoPlatformCreate(&config, &servo->platform);
        if (status != kModServoStatusOK) {
                c_free(servo);
                xsServoThrowStatus(the, status);
                return;
        }

        xsmcSetHostData(xsThis, servo);
}

void xs_servo_close(xsMachine *the)
{
        xsServo servo = xsmcGetHostData(xsThis);
        if (!servo)
                return;

        xs_servo_destructor(servo);
        xsmcSetHostData(xsThis, NULL);
}

void xs_servo_write(xsMachine *the)
{
        xsServo servo = xsmcGetHostData(xsThis);
        modServoStatus status;

        if (!servo || !servo->platform)
                xsUnknownError("closed");

        status = modServoPlatformWriteDegrees(servo->platform, xsmcToNumber(xsArg(0)));
        if (status != kModServoStatusOK)
                xsServoThrowStatus(the, status);
}

void xs_servo_writeMicroseconds(xsMachine *the)
{
        xsServo servo = xsmcGetHostData(xsThis);
        modServoStatus status;

        if (!servo || !servo->platform)
                xsUnknownError("closed");

        status = modServoPlatformWriteMicros(servo->platform, xsmcToNumber(xsArg(0)));
        if (status != kModServoStatusOK)
                xsServoThrowStatus(the, status);
}
