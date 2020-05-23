#include "cap.h"
#include "common.h"
#include "child.h"
#include "param.h"
#include "mount_ns.h"
#include "seccomp_filter.h"

int child_exec(void *stuff) {

  if (-1 == prctl(PR_SET_PDEATHSIG, SIGKILL)) {
    log_error("Cannot set PR_SET_DEATHSIG for child process: %m");
    return EXIT_FAILURE;
  }

  exec_param_t *params = (exec_param_t *) stuff;

  if (-1 == sethostname(params->utsname, strlen(params->utsname))) {
    log_warn("Error changing hostname: %m");
  }

  if (-1 == change_root_and_mount(params)) {
    return EXIT_FAILURE;
  }

  if (params->sandbox && (-1 == set_container_cap())) {
    return EXIT_FAILURE;
  }

  if (params->seccomp && (-1 == enable_seccomp())) {
    return EXIT_FAILURE;
  }

  if (-1 == execve(params->program_name, params->argv, (char * const *) params->env.ptrs)) {
    PANIC("Error executing %s: %m", params->argv[0]);
  }

  log_error("Your code shouldn't reach here.");

  return EXIT_FAILURE;
}
