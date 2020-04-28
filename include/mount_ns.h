#ifndef MOUNT_H
#define MOUNT_H 1

int change_root(const char *newroot);
int host_mount(const char *hostpath, const char *mountpath);

#endif /* MOUNT_H */