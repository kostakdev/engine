
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

int main(int argc, char **argv) {
  const size_t page_size = sysconf(_SC_PAGE_SIZE);
  const size_t HEAP_STACK_SIZE = aligned_size(KOSTAK_LIMITED_STACK_SIZE + UMM_MALLOC_CFG_HEAP_SIZE, page_size);
  uint8_t *heap_stack =  mmap(NULL, HEAP_STACK_SIZE, PROT_READ | PROT_WRITE, 
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

  UMM_MALLOC_CFG_HEAP_ADDR = heap_stack + KOSTAK_LIMITED_STACK_SIZE;
  uint8_t *stack_top = UMM_MALLOC_CFG_HEAP_ADDR;

  if (NULL == heap_stack) {
    PANIC("Error allocating space for heap using mmap: %m");
  } else {
    log_debug("Allocated heap and stack space: %lu at address %p", 
        (unsigned long) HEAP_STACK_SIZE,
        heap_stack);
  }
   
  exec_param_t params;
  memset(&params, 0, sizeof(exec_param_t));
  parse_arg(argc, argv, &params);

  log_set_level(params.log_level);
  
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

  const pid_t child_pid = clone(child_exec, stack_top, clone_flags, (void*) &params);

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

