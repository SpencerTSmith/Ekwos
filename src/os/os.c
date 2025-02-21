#include "os/os.h"

#ifdef OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <unistd.h>
#endif

void os_sleep_ms(u32 milliseconds) {
#ifdef OS_WINDOWS
  Sleep(milliseconds);
#elif OS_LINUX
  usleep(milliseconds * 1000);
#endif
}
