#include "common.h"
#include "env.h"

static const size_t env_init_cap = 256;
void init_env_buf(env_var_t* e) {
  memset(e, 0, sizeof(env_var_t));

  struct rlimit lim;
  size_t stack_size = 1024;
  if (0 == getrlimit(RLIMIT_STACK, &lim)) {
    stack_size = lim.rlim_cur / 4;
  }

  e->maxlen = stack_size;
  e->cap = env_init_cap;

  e->buf = mmap(NULL, stack_size, PROT_READ | PROT_WRITE,
    MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  e->bufarr = mmap(NULL, e->cap * sizeof(char *), PROT_READ | PROT_WRITE,
    MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  e->next = e->buf;
}

void add_env(env_var_t *e, const char *env_var_str) {
  const size_t bufsize = strlen(env_var_str) + 1;

  if (e->len + bufsize > e->maxlen) {
    log_warn("You're exceeding the maximum length of environment variable");
    return;
  }

  memmove(e->next, env_var_str, bufsize);

  if (++e->count > e->cap) {
    mremap(e->bufarr, e->cap * sizeof(char *), 
    e->cap * 2 * sizeof(char*),
    MREMAP_MAYMOVE);
    e->cap *= 2;
  }

  e->bufarr[e->count - 1] = e->next;
  e->next = e->next + bufsize;
  e->len += bufsize;
}

void clear_env_buf(env_var_t* e) {
  munmap(e->buf, e->maxlen);
  munmap(e->bufarr, (e->len <= 256 ? 256 : e->len) * sizeof(char *));
  memset(e, 0, sizeof(env_var_t));
}