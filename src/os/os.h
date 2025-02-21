#ifndef OS_H
#define OS_H

#include "core/common.h"

/* NOTE(ss): Since so far this is the only thing we need specific to each platform,
 * I thought to keep it simple and just do definition based implementations, if we go further and
 * need to separate these into specific translations units and do the whole conditionally compiling
 * thing, we may */

void os_sleep_ms(u32 milliseconds);

#endif // OS_H
