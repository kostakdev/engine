#include "common.h"
#include "util.h"

void init_ptr_vec(ptr_vec_t *vec, void **init_values, const size_t count)
{
  memset(vec, 0, sizeof(ptr_vec_t));
  vec->ptrs = mmap(0, INIT_SIZE * sizeof(void*), PROT_WRITE | PROT_READ, 
    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);  
  vec->cap = INIT_SIZE;
  
  if (init_values != NULL && count > 0) {
    memmove(vec->ptrs, init_values, count * sizeof(void*));
    vec->count = count;
  }
}

int add_ptr(ptr_vec_t *vec, void *ptr)
{
  if (vec->cap == vec->count) {
    const size_t newcap = vec->cap + INIT_SIZE;
    void *newbuf = mremap(vec->ptrs, vec->cap * sizeof(void*),
      newcap * sizeof(void*), MREMAP_MAYMOVE);

    if (NULL == newbuf) {
      log_error("Failed when remapping buffer: %m");
      return -1;
    }

    vec->cap = newcap;
    vec->ptrs = newbuf;
  }

  vec->ptrs[vec->count++] = ptr;
  return 0;
}

void free_ptr_vec(ptr_vec_t *vec) 
{
  munmap(vec->ptrs, vec->cap * sizeof(void*));
  memset(vec, 0, sizeof(ptr_vec_t));
}
