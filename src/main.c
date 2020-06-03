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

#include "child.h"
#include "common.h"
#include "net_eth.h"
#include "param.h"

/* This program only works on kernel 3.13 and up */
static bool check_kernel_version() {
  struct utsname buffer;

  if (-1 == uname(&buffer)) {
    log_error("Cannot get uname.");
    return false;
  }

  log_debug("Kernel release: %s", buffer.release);

  /* xx.x.x */
  char *p = buffer.release;
  char *dot;
  int major = strtoul(p, &dot, 10);
  int minor = strtoul(dot + 1, NULL, 10);

  bool verdict = major > 3 || (major == 3 && minor > 13);

  log_debug("Parsed version %d.%d", major, minor);
  if (!verdict) {
    log_warn("This program only supported on linux 3.13 and up. Your linux is: %d.%d", major, minor);
  }

  return verdict;
}

/* Set the stack size to musl's limit */
#define KOSTAK_LIMITED_STACK_SIZE (80 * KBYTES)

const size_t UMM_MALLOC_CFG_HEAP_SIZE = (128 * KBYTES);
void *UMM_MALLOC_CFG_HEAP_ADDR = NULL;

static inline size_t aligned_size(const size_t sz, const size_t alignment) {
  return ((sz + (alignment - 1)) & ~(alignment -1));
} 

static ssize_t allocate_heap_stack(const size_t stack_size, 
                               const size_t heap_size,
                               void **base, 
                               void **heap_base) 
{
  const size_t page_size = sysconf(_SC_PAGE_SIZE);
  const size_t aligned_stack_size = aligned_size(stack_size, page_size);
  const size_t aligned_heap_size = aligned_size(heap_size, page_size);
  
  uint8_t *base_addr = mmap(NULL, aligned_stack_size + aligned_heap_size, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  if (NULL == base_addr) {
    log_error("Failure allocating stack and heap: %m");
    return -1;
  }
  *base = (void*) base_addr; 
  *heap_base = (void*) (base_addr + aligned_stack_size);
  return aligned_stack_size + aligned_heap_size;
}

int main(int argc, char **argv) {
  log_set_level(LOG_WARN);

  uint8_t *stack;
  ssize_t memsize;
  
  if (0 > (memsize = allocate_heap_stack(KOSTAK_LIMITED_STACK_SIZE, UMM_MALLOC_CFG_HEAP_SIZE,
        (void**) &stack, (void**) &UMM_MALLOC_CFG_HEAP_ADDR))) 
  {
    return EXIT_FAILURE;
  }

  exec_param_t params;
  memset(&params, 0, sizeof(exec_param_t));
  parse_arg(argc, argv, &params);

  log_set_level(params.log_level);
 
  log_debug("Allocated %lu as heap and stack: stack %p | heap %p", memsize, stack, UMM_MALLOC_CFG_HEAP_ADDR);

  if(!check_kernel_version()) {
    exit(EXIT_FAILURE);
  }

  static size_t child_stack_size = KOSTAK_LIMITED_STACK_SIZE;

  log_debug("Stack Size Allocated: %ld KB", child_stack_size / KBYTES);
 
  const int clone_flags = SIGCHLD
    | CLONE_NEWNS
    | CLONE_NEWPID
    | CLONE_NEWNET
    | CLONE_NEWUTS;

  /* bottom of heap = top of the stack */
  const pid_t child_pid = clone(child_exec, UMM_MALLOC_CFG_HEAP_ADDR, clone_flags, (void*) &params);

  if (-1 == child_pid) {
    PANIC("Error when cloning: %m\n");
  }

  log_debug("Started, parent %ld, child %ld", (long) getpid(), (long) child_pid);

  if (-1 == prepare_netns(child_pid, &params)) {
    PANIC("Error preparing network namespace: %m");
  }
  int child_wstatus;
  if (-1 == waitpid(child_pid, &child_wstatus, 0)) {
    PANIC("Error when waiting child process: %m\n");
  }

  if (!WIFEXITED(child_wstatus)) {
    PANIC("Child Failed status=%ld, !WIFEXITED!", child_wstatus);
  }

  int child_status = WEXITSTATUS(child_wstatus);

  if (child_status != 0) {
    log_fatal("Child Failed with status=%d", child_status);
    return child_status;
  }

  log_debug("Process %ld terminated", (long) child_pid);
  return EXIT_SUCCESS;
}

