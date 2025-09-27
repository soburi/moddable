#ifndef MOD_RESOURCE_PLATFORM_H
#define MOD_RESOURCE_PLATFORM_H

#include "xs.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const void *modResourcePlatformLookup(xsMachine *the, const char *path, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* MOD_RESOURCE_PLATFORM_H */
