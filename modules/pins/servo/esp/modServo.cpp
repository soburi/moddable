/*
 * ESP8266 servo hardware implementation using Arduino Servo library.
 */

#include "servo_platform.h"

#include "xsHost.h"

#include "Servo.h"

#include <new>
#include <stdint.h>

struct modServoPlatformRecord {
        Servo *servo;
        int min;
        int max;
        uint8_t running;
};

typedef struct modServoPlatformRecord modServoPlatformRecord;

modServoStatus modServoPlatformCreate(const modServoConfiguration *config, modServoPlatform *outServo)
{
        modServoPlatformRecord *record;
        Servo *servo;

        if (!config || !outServo)
                return kModServoStatusInvalidArgument;

        record = (modServoPlatformRecord *)c_calloc(1, sizeof(modServoPlatformRecord));
        if (!record)
                return kModServoStatusNoMemory;

        record->min = config->hasMin ? (int)config->min : MIN_PULSE_WIDTH;
        record->max = config->hasMax ? (int)config->max : MAX_PULSE_WIDTH;

        servo = new (std::nothrow) Servo;
        if (!servo) {
                c_free(record);
                return kModServoStatusNoMemory;
        }

        servo->attach(config->pin, record->min, record->max);

        record->servo = servo;
        record->running = true;

        *outServo = record;
        return kModServoStatusOK;
}

void modServoPlatformClose(modServoPlatform platform)
{
        modServoPlatformRecord *record = (modServoPlatformRecord *)platform;

        if (!record || !record->running)
                return;

        delete record->servo;
        record->servo = nullptr;
        record->running = false;
}

void modServoPlatformDelete(modServoPlatform platform)
{
        modServoPlatformRecord *record = (modServoPlatformRecord *)platform;
        if (!record)
                return;

        modServoPlatformClose(record);
        c_free(record);
}

modServoStatus modServoPlatformWriteDegrees(modServoPlatform platform, double degrees)
{
        modServoPlatformRecord *record = (modServoPlatformRecord *)platform;
        int microseconds;

        if (!record || !record->running || !record->servo)
                return kModServoStatusNotOpen;

        microseconds = (int)(((double)(record->max - record->min) * degrees) / 180.0) + record->min;
        record->servo->writeMicroseconds(microseconds);

        return kModServoStatusOK;
}

modServoStatus modServoPlatformWriteMicros(modServoPlatform platform, double microseconds)
{
        modServoPlatformRecord *record = (modServoPlatformRecord *)platform;

        if (!record || !record->running || !record->servo)
                return kModServoStatusNotOpen;

        record->servo->writeMicroseconds((int)microseconds);
        return kModServoStatusOK;
}
