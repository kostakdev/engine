
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


int main(int argc, char **argv) {
  
  exec_param_t params;
  memset(&params, 0, sizeof(exec_param_t));
  parse_arg(argc, argv, &params);

  log_set_level(params.log_level);
  
  if(!check_kernel_version()) {
    exit(EXIT_FAILURE);
  }

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
    | CLONE_NEWPID
    | CLONE_NEWNET
    | CLONE_NEWUTS;

  const pid_t child_pid = clone(child_exec, stack_top, clone_flags, (void*) &params);

  if (-1 == child_pid) {
    PANIC("Error when cloning: %m\n");
  }

  log_debug("Started, parent %ld, child %ld", (long) getpid(), (long) child_pid);

  if (-1 == prepare_netns(child_pid, &params)) {
    PANIC("Error preparing network namespace: %m");
  }
  if (-1 == waitpid(child_pid, NULL, 0)) {
    PANIC("Error when waiting child process: %m\n");
  }

  log_debug("Process %ld terminated", (long) child_pid);
  return EXIT_SUCCESS;
}

