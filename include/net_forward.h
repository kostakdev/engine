/* * Kostak - A little container engine
 * Copyright (C) 2020 Didiet Noor <dnoor@ykode.com>
 *
 * This file is part of Kostak.
 *
 * Kostak is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Kostak is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Kostak.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_FORWARD
#define NET_FORWARD  1

typedef enum nat_hook_chain{
  NAT_PREROUTING = NF_INET_PRE_ROUTING,
  NAT_POSTROUTING = NF_INET_POST_ROUTING,
  NAT_OUTPUT = NF_INET_LOCAL_OUT,
} nat_hook_chain_t;

int start_nf_batch(void *buf, void **nextbuf);

int end_nf_batch(void *buf, void **nextbuf);

int flush_rules(void *buf, void **nextbuf);

int create_table(const char *table_name, void *buf, void **nextbuf);

int delete_table(const char *table_name);

int create_nat_chain(const char *table_name, const char *chain_name,
                     const nat_hook_chain_t hook_type,
                     void *buf, void **nextbuf);

int create_masq_rule(const char *table_name, const char *chain_name, 
                        void *buf, void **nextbuf);

int create_tcp_portforward_rule(const char *table_name, const char *chain_name,
  const __u16 dest_port, const char *target_addr, const __u16 target_port, 
  void *buf, void **nextbuf);


#endif /* NET_FORWARD */
