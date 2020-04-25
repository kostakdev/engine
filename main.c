
#include "child.h"
#include "common.h"
#include "param.h"

int main(int argc, char **argv) {
  exec_param_t params;
  memset(&params, 0, sizeof(exec_param_t));
  parse_arg(argc, argv, &params);

  log_set_level(params.log_level);
  
  struct rlimit lim;
  static size_t child_stack_size = 1024 * KBYTES;

  if (0 == getrlimit(RLIMIT_STACK, &lim)) {
    child_stack_size = lim.rlim_cur / 4;
  }

  log_debug("Stack Size Allocated: %ld KB", child_stack_size / KBYTES);

  uint8_t *stack = mmap(NULL, child_stack_size, PROT_READ | PROT_WRITE,
    MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  
  uint8_t *stack_top = stack + child_stack_size;

  const int clone_flags = SIGCHLD
    | CLONE_NEWNS
    | CLONE_NEWUTS;

  const pid_t child_pid = clone(child_exec, stack_top, clone_flags, (void*) &params);

  if (-1 == child_pid) {
    PANIC("Error when cloning: %m\n");
  }

  log_debug("Started, parent %ld, child %ld", (long) getpid(), (long) child_pid);

  if (-1 == waitpid(child_pid, NULL, 0)) {
    PANIC("Error when waiting child process: %m\n");
  }

  log_debug("Process %ld terminated", (long) child_pid);
  return EXIT_SUCCESS;
}

