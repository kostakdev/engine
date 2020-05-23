#include <seccomp.h>
#include "common.h"
#include "seccomp_filter.h"
#include "seccomp_filter_internal.h"

#ifdef SCMP_ACT_KILL_PROCESS
#define BLOCK_ACTION SCMP_ACT_KILL_PROCESS
#else
#define BLOCK_ACTION SCMP_ACT_KILL
#endif

#define ALLOW(x) \
do { \
  rc = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, \
      seccomp_syscall_resolve_name((x)), 0); \
  if (rc < 0) { \
    goto ret; \
  } \
  log_debug(" %s", x); \
} while (0)

int enable_seccomp() {
  int rc = -1;

  scmp_filter_ctx ctx = seccomp_init(BLOCK_ACTION);
  
  if (NULL == ctx) {
    goto ret;
  }

  log_debug("SecComp: the following syscalls will be allowed by seccomp:");
  
  int i;
  for (i = 0; i < (int) (sizeof(syscalls) / sizeof(char *)); ++i) {
    ALLOW(syscalls[i]);
  }

  rc = seccomp_load(ctx);

ret:
  seccomp_release(ctx);
  return rc;
}
