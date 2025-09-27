#include "resource_platform.h"
#include "mc.xs.h"

extern const void* mcGetResource(xsMachine* the, char* path, size_t* size);

const void *modResourcePlatformLookup(xsMachine *the, const char *path, size_t *size)
{
        return mcGetResource(the, (char *)path, size);
}
