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

#include "common.h"
#include "util.h"

void init_ptr_vec(ptr_vec_t *vec, void **init_values, const size_t count)
{
  memset(vec, 0, sizeof(ptr_vec_t));
  vec->ptrs = umm_malloc(INIT_SIZE * sizeof(void*));
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
    void *newbuf = umm_realloc(vec->ptrs, newcap * sizeof(void*));
    
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
  umm_free(vec->ptrs);
  memset(vec, 0, sizeof(ptr_vec_t));
}
