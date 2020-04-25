#include "common.h"
#include "child.h"
#include "param.h"
#include "mount_ns.h"

int child_exec(void *stuff) {
  exec_param_t *params = (exec_param_t *) stuff;
  char * const envp[] = {
    "PS1=\\u@\\h:\\w\\$ ",
    "PATH=/bin:/sbin:/usr/bin/:/usr/sbin:/usr/local/bin",
    NULL
  };

  if (-1 == sethostname(params->utsname, strlen(params->utsname))) {
    log_warn("Error changing hostname: %m");
  }

  if (-1 == change_root(params->rootfs)) {
    return EXIT_FAILURE;
  }

  if (-1 == execve(params->program_name, params->argv, envp)) {
    PANIC("Error executing %s: %m", params->argv[0]);
  }

  log_error("Your code shouldn't reach here.");

  return EXIT_FAILURE;
}