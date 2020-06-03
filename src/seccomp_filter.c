/* * Kostak - A little container engine
 * Copyright (C) 2020 Didiet Noor <dnoor@ykode.com>
 *
 * This file is part of Kostak.
 *
 * Kostak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Kostak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Kostak.  If not, see <http://www.gnu.org/licenses/>.
 */

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
