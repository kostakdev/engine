#include "common.h"
#include "cap.h"

int set_container_cap() {
  int ret;

  ret = prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0);

  if (ret < 0) {
    log_error("Error prctl(PR_SET_NO_PRIVS): %m");
    return ret;
  }

  ret = prctl(PR_CAP_AMBIENT, PR_CAP_AMBIENT_CLEAR_ALL, 0, 0, 0);

  if (ret < 0) {
    log_error("Error prctl(PR_CAP_AMBIENT): %m");
    return ret;
  }

  int i;

  for(i = 0;; ++i) {
    if (CAP_NET_BIND_SERVICE == i) {
      continue;
    }

    ret = prctl(PR_CAPBSET_DROP, i, 0, 0, 0);
    if (ret < 0) {
      if (errno == EINVAL) {
        break;
      }
      log_error("prctl (PR_CAPBSET_DROP): %m");
      return ret;
    }
  }
  
  struct __user_cap_header_struct hdr = { _LINUX_CAPABILITY_VERSION_3, 0 };
  struct __user_cap_data_struct data[2] = { {0} };

  data[0].effective |= 1 << CAP_NET_BIND_SERVICE;
  data[0].permitted |= 1 << CAP_NET_BIND_SERVICE;
  data[0].inheritable |= 1 << CAP_NET_BIND_SERVICE;
  
  ret = capset(&hdr, data);
  
  if (ret < 0) {
    log_error("capset: %m");
    return ret;
  } 

  return 0;
  
}


