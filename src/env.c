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
  e->capsize = 1024;
  e->cap = env_init_cap;

  e->buf = mmap(NULL, e->capsize, PROT_READ | PROT_WRITE,
    MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  e->bufarr = mmap(NULL, e->cap * sizeof(char *), PROT_READ | PROT_WRITE,
    MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

  e->next = e->buf;
}

static inline size_t map_aligned(const size_t sz) {
  return sz & ~(sysconf(_SC_PAGE_SIZE) - 1);
}

int compact_env(env_var_t *e) {
  if (e->len == e->maxlen) {
    return 0; /* already compact enough */
  }

  void *newbuf = mremap(e->buf, e->capsize, map_aligned(e->len), MREMAP_MAYMOVE);
  if (newbuf == NULL) {
    log_error("Cannot re-map the new buffer: %m");
    return -1;
  }
  e->buf = newbuf;
  e->capsize = map_aligned(e->len);

  log_debug("Compacted env buffer to %ld.", (long) e->capsize);
  return 0;
}

void add_env(env_var_t *e, const char *env_var_str) {
  const size_t bufsize = strlen(env_var_str) + 1;
  const size_t newsize = map_aligned(e->len + bufsize);

  if (newsize > e->maxlen) {
    log_warn("You're exceeding the maximum length of environment variable");
    return;
  }

  if (newsize > e->capsize) {
    const size_t newcap = e->capsize + 1024;
    const size_t next_offset = e->next - e->buf;
    void *newbuf = mremap(e->buf, e->capsize, newcap, MREMAP_MAYMOVE);

    if (newbuf == NULL) {
      log_error("Cannot resize the environment variable buffer: %m");
      return;
    }
    e->buf = newbuf;
    e->capsize = newcap;
    e->next = e->buf + next_offset;
  }

  memmove(e->next, env_var_str, bufsize);

  if (++e->count > e->cap) {
    void *newarr = mremap(e->bufarr, e->cap * sizeof(char *), 
      e->cap * 2 * sizeof(char*),
      MREMAP_MAYMOVE);
    if (newarr == NULL) {
      log_error("Cannot resize array: %m");
      return;
    }
    e->bufarr = newarr;
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