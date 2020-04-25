#include "common.h"
#include "mount_ns.h"

int change_root(const char *newroot) {
  if (-1 == mount("", "/", "",
            MS_REC
            | MS_SLAVE,
            NULL)) 
  {
    log_error("Failed to mount / as slave mount: %m");
    return -1;
  }

  char mount_point[] = "/tmp/kostak.rootfs.XXXXXX";
  if( NULL == mkdtemp(mount_point) ) {
    log_error("Error creating mount point at /tmp: %m");
    return -1;
  }

  log_debug("Created mount point at %s", mount_point);

  if( -1 == mount(newroot, mount_point, "", MS_REC | MS_BIND, NULL)) {
    log_error("Error mounting %s to %s: %m", newroot, mount_point);
    rmdir(mount_point);
    return -1;
  }

  log_debug("%s mounted to %s", newroot, mount_point);

  char put_old[] = "/tmp/kostak.rootfs.XXXXXX/put_old.XXXXXX";
  memmove(put_old, mount_point, strlen(mount_point));

  if( NULL == mkdtemp(put_old) ) {
    log_error("Error creating mount point at %s: %m", put_old);
    umount2(mount_point, MNT_DETACH);
    rmdir(mount_point);
    return -1;
  }

  log_debug("%s created for putting old rootfs", put_old);

  if(-1 == syscall(SYS_pivot_root, mount_point, put_old)) {
    log_error("Failure pivoting root to %s: %m", mount_point);
    rmdir(put_old);
    umount2(mount_point, MNT_DETACH);
    rmdir(mount_point);
    return -1;
  }

  log_debug("Pivoting to %s succeeded", mount_point);

  if (-1 == chdir("/")) {
    log_error("Failure on changing working dir to new root: %m");
    return -1;
  }

  char base_old_dir[PATH_MAX];
  char *old_dir = basename(put_old);

  strncpy(base_old_dir, old_dir, PATH_MAX);

  if (-1 == umount2(old_dir, MNT_DETACH)) {
    log_error("Failure unmounting %s: %m", old_dir);
    return -1;
  }

  if (-1 == rmdir(old_dir)) {
    log_error("Failure removing %s: %m", old_dir);
    return -1;
  }

  log_debug("Cleaned up %s succeeded", old_dir);

  return 0;
}