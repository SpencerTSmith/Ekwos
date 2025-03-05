#include "os/os.h"

#ifdef OS_WINDOWS
#include <windows.h>
#elif OS_LINUX
#include <unistd.h>
#endif

void os_sleep_ms(u64 milliseconds) {
#ifdef OS_WINDOWS
  Sleep(milliseconds);
#elif OS_LINUX
  usleep(milliseconds * 1000);
#endif
}

void os_sleep_ns(u64 nanoseconds) {
#ifdef OS_WINDOWS
  Sleep(milliseconds);
#elif OS_LINUX
  usleep(nanoseconds / 1e3);
#endif
}
