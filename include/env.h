#ifndef ENV_H
#define ENV_H 1
#include <stdint.h>

struct env_var {
  char *buf;        /* character buffer */
  char *next;       /* next free buffer */
  char **bufarr;    /* array of pointer inside buf */

  size_t cap;       /* capacity count of bufarr */
  size_t count;     /* environment variable count */

  size_t len;       /* current length */
  size_t maxlen;    /* maximum length */
  size_t capsize;   /* current capacity */
};

typedef struct env_var env_var_t;

void init_env_buf(env_var_t* e);

void add_env(env_var_t *e, const char *env_var_str);

int compact_env(env_var_t *e);

void clear_env_buf(env_var_t* e);

#endif /* ENV_H */