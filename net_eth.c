#include "net_eth.h"
#include "netlink_util.h"

struct nl_ireq {
  struct nlmsghdr n;
  struct ifinfomsg i;
};

static const unsigned int UNUSED_IFI_CHANGE = 0xFFFFFFFFu;

int create_veth(const int sock_fd, const char *ifname, 
                const char *peername, const pid_t child_pid)
{
  const __u16 flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
  uint8_t buffer[PAYLOAD_MAX];
  memset(buffer, 0, PAYLOAD_MAX);

  struct nl_ireq *req = (struct nl_ireq *) buffer;
  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req->n.nlmsg_flags = flags;
  req->n.nlmsg_type = RTM_NEWLINK;
  req->i.ifi_family = PF_NETLINK;
  req->i.ifi_change = UNUSED_IFI_CHANGE;


  struct nlmsghdr *n = &req->n;
  const int maxlen = PAYLOAD_MAX;
  addattr_string(n, maxlen, IFLA_IFNAME, ifname);
  struct rtattr *linkinfo = addattr_nest(n, maxlen, IFLA_LINKINFO);
  addattr_string(n, maxlen, IFLA_INFO_KIND, "veth");
  struct rtattr *infodata = addattr_nest(n, maxlen, IFLA_INFO_DATA);
  struct rtattr *vethinfo = addattr_nest(n, maxlen, VETH_INFO_PEER);

  struct ifinfomsg *peerinfo = reserve_space(n, maxlen, sizeof(struct ifinfomsg));
  if (NULL == peerinfo) {
    log_error("Error reserving space: %m");
    return -1;
  }

  peerinfo->ifi_family = IFA_UNSPEC;
  peerinfo->ifi_change = UNUSED_IFI_CHANGE;

  addattr_string(n, maxlen, IFLA_IFNAME, peername);
  addattr_uint32(n, maxlen, IFLA_NET_NS_PID, child_pid);
  addattr_nest_end(n, vethinfo); 
  addattr_nest_end(n, infodata);
  addattr_nest_end(n, linkinfo);

  return send_nlmsg(sock_fd, (struct nlmsghdr *)buffer, true);
}

int prepare_netns(const pid_t child_pid) {
  int status = 0;
  const int sock_fd = create_netlink_route_socket();

  status = create_veth(sock_fd, "kostak0", "kostak1", child_pid);

  close(sock_fd);

  return status;
}