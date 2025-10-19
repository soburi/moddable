#include <zephyr/kernel.h>

#include "xsmain.h"

__attribute__((weak)) int main(void)
{
        xs_setup();
        return 0;
}
