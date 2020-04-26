#include "common.h"
#include "netlink_util.h"
#include "net_forward.h"

const size_t TABLE_CLEAR_SIZE = sizeof(struct nlmsghdr) + sizeof(struct nfgenmsg);

int start_nf_batch(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST;
  req->n.nlmsg_type = NFNL_MSG_BATCH_BEGIN;
  req->n.nlmsg_seq = time(NULL);

  req->f.res_id = NFNL_SUBSYS_NFTABLES;

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;
}

int end_nf_batch(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST;
  req->n.nlmsg_type = NFNL_MSG_BATCH_END;
  req->n.nlmsg_seq = time(NULL);

  req->f.res_id = NFNL_SUBSYS_NFTABLES;

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;
}

int flush_rules(void *buf, void **nextbuf) {
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_DELTABLE;
  req->n.nlmsg_seq = time(NULL);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0; 
}

int create_table(const char *table_name, void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWTABLE;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  addattr_string(&req->n, PAYLOAD_MAX, NFTA_TABLE_NAME, table_name);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
}

int create_nat_chain(const char *table_name, const char *chain_name,
                     const nat_hook_chain_t hook_type,
                     void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWCHAIN;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  struct nlmsghdr *n = &req->n;

  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_TABLE, table_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_NAME, chain_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_CHAIN_TYPE, "nat");
  addattr_uint32(n, PAYLOAD_MAX, NFTA_CHAIN_POLICY, htonl(NF_ACCEPT));

  struct rtattr *hookdata = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_CHAIN_HOOK);
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_HOOKNUM, htonl(hook_type));
  addattr_uint32(n, PAYLOAD_MAX, NFTA_HOOK_PRIORITY, htonl(100));
  addattr_nest_end(n, hookdata);

  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;    
}

int create_masq_rule(const char *table_name, const char *chain_name, 
                        void *buf, void **nextbuf)
{
  struct nl_req *req = (struct nl_req *) buf;
  memset(buf, 0, TABLE_CLEAR_SIZE);

  req->n.nlmsg_len = NLMSG_LENGTH(sizeof(struct nfgenmsg));
  req->n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
  req->n.nlmsg_type = NFNL_SUBSYS_NFTABLES << 8 | NFT_MSG_NEWRULE;
  req->n.nlmsg_seq = time(NULL);

  req->f.nfgen_family = AF_INET;

  struct nlmsghdr *n = &req->n;

  addattr_string(n, PAYLOAD_MAX, NFTA_RULE_TABLE, table_name);
  addattr_string(n, PAYLOAD_MAX, NFTA_RULE_CHAIN, chain_name);

  struct rtattr *rule = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_RULE_EXPRESSIONS);
  struct rtattr *elem_masq = addattr_nest(n, PAYLOAD_MAX, NLA_F_NESTED | NFTA_LIST_ELEM);
  addattr_string(n, PAYLOAD_MAX, NFTA_EXPR_NAME, "masq");
  addattr_nest_end(n, elem_masq);
  addattr_nest_end(n, rule);
  if (nextbuf) {
    *nextbuf = (uint8_t *) buf + req->n.nlmsg_len;
  }
  return 0;  
}