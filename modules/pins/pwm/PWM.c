/*
 * PWM XS bindings delegating hardware operations to platform implementations.
 */

#include "xsmc.h"
#include "mc.xs.h"
#include "xsHost.h"

#include "pwm_platform.h"

typedef struct {
        modPWMPlatform platform;
} xsPWMRecord, *xsPWM;

static void xsPWMThrowStatus(xsMachine *the, modPWMStatus status)
{
        switch (status) {
        case kModPWMStatusOK:
                break;
        case kModPWMStatusInvalidArgument:
                xsRangeError("invalid pwm configuration");
                break;
        case kModPWMStatusChannelUnavailable:
                xsUnknownError("no pwm channels available");
                break;
        case kModPWMStatusNoMemory:
                xsUnknownError("no memory");
                break;
        case kModPWMStatusHardwareError:
        default:
                xsUnknownError("pwm hardware error");
                break;
        }
}

void xs_pwm_destructor(void *data)
{
        xsPWM pwm = data;
        if (!pwm)
                return;

        if (pwm->platform) {
                modPWMPlatformDelete(pwm->platform);
                pwm->platform = NULL;
        }

        c_free(pwm);
}

void xs_pwm(xsMachine *the)
{
        xsPWM pwm = c_calloc(1, sizeof(xsPWMRecord));
        modPWMSetup setup = {0};
        modPWMStatus status;

        if (!pwm)
                xsUnknownError("no memory");

        if (!xsmcTest(xsArg(0))) {
                c_free(pwm);
                xsUnknownError("options required");
        }

        xsmcVars(1);
        if (!xsmcHas(xsArg(0), xsID_pin)) {
                c_free(pwm);
                xsUnknownError("pin missing");
        }

        xsmcGet(xsVar(0), xsArg(0), xsID_pin);
        setup.pin = xsmcToInteger(xsVar(0));

        if (xsmcHas(xsArg(0), xsID_port)) {
                xsmcGet(xsVar(0), xsArg(0), xsID_port);
                setup.hasPort = true;
                setup.port = xsmcToInteger(xsVar(0));
        }

        status = modPWMPlatformCreate(&setup, &pwm->platform);
        if (status != kModPWMStatusOK) {
                c_free(pwm);
                xsPWMThrowStatus(the, status);
                return;
        }

        xsmcSetHostData(xsThis, pwm);
}

void xs_pwm_close(xsMachine *the)
{
        xsPWM pwm = xsmcGetHostData(xsThis);
        if (!pwm)
                return;

        xs_pwm_destructor(pwm);
        xsmcSetHostData(xsThis, NULL);
}

void xs_pwm_write(xsMachine *the)
{
        xsPWM pwm = xsmcGetHostData(xsThis);
        modPWMStatus status;

        if (!pwm || !pwm->platform)
                xsUnknownError("closed");

        status = modPWMPlatformWrite(pwm->platform, xsmcToInteger(xsArg(0)));
        if (status != kModPWMStatusOK)
                xsPWMThrowStatus(the, status);
}
