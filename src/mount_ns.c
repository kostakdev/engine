#include "common.h"
#include "mount_ns.h"
#include "util.h"

static inline int mount_proc() 
{
  int e;
  if (-1 == (e = mkdir("/proc", 0555))) {
    if (EEXIST == errno) {
      log_debug("/proc directory exists, skipping");
    } else {
      log_error("Error creating /proc directory: %m");
      return -1;
    }
  }

  if (-1 == mount("proc", "/proc", "proc", 0, "")) {
    log_error("Error mounting proc filesystem: %m");
    return -1;
  }

  return 0;
}

int host_mount(const char *host_mount, const char *target_mount)
{
  char host_real_path[PATH_MAX];
  char target_real_path[PATH_MAX];

  if (NULL == realpath(host_mount, host_real_path)) {
    log_error("Cannot find a real path for %s: %m", host_mount);
    return -1;
  }

  if (NULL == realpath(target_mount, target_real_path)) {
    log_error("Cannot find a real path for %s: %m", target_mount);
    return -1;
  }

  log_debug("Attempting mounting %s to %s", host_real_path, target_real_path);

  if (-1 == mount(host_real_path, target_real_path, "", MS_REC|MS_BIND, NULL)) {
    log_error("Failed mounting host path");
    return -1;
  }

  return 0;
}

static int mount_mapping(const char *newroot, ptr_vec_t *mounts) 
{
  size_t i;
  char target_fullpath[PATH_MAX];
  for(i = 0; i < mounts->count; ++i) {
    const char *mountmap = (const char*) mounts->ptrs[i];
    char *colon = strchr(mountmap, ':');
    *colon = '\0';
    snprintf(target_fullpath, PATH_MAX, "%s/%s", newroot, colon + 1);
    host_mount(mountmap, target_fullpath);
    *colon = ':';
  }

  return i;
}

int change_root_and_mount(exec_param_t *param)
{
  const char *newroot = param->rootfs;
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

  mount_mapping(mount_point, &param->mounts);

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

  if (-1 == mount_proc()) {
    return -1;
  }

  if (-1 == mount("tmpfs", "/tmp", "tmpfs", 0, "")) {
    log_error("Failure on mounting tmpfs: %m");
    return -1;
  }

  char base_old_dir[] = "put.old.XXXXXX";
  char *old_dir = basename(put_old);

  memmove(base_old_dir, old_dir, sizeof(base_old_dir));

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
