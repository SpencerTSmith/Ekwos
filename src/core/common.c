#include "core/common.h"
#include <time.h>

f64 get_time_s(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

u64 get_time_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (ts.tv_sec * 1e3) + (ts.tv_nsec * 1e-6);
}
