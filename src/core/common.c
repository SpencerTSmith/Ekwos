#include "core/common.h"
#include <time.h>

f64 get_time_s(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return ts.tv_sec + (ts.tv_nsec / NSEC_PER_SEC);
}

u64 get_time_ms(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return (ts.tv_sec * MSEC_PER_SEC) + (ts.tv_nsec / 1e-6);
}

u64 get_time_ns(void) {
  struct timespec ts;
  timespec_get(&ts, TIME_UTC);
  return (ts.tv_sec * NSEC_PER_SEC) + (ts.tv_nsec);
}
