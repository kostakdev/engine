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

#ifndef KOSTAK_UTIL_H
#define KOSTAK_UTIL_H 1

typedef struct ptr_vec {
  void **ptrs;    /* Array of pointers */
  size_t count;   /* Total count of the pointers */
  size_t cap;     /* Total capacity of the array of pointers */
} ptr_vec_t;

static const size_t INIT_SIZE = 256;

void init_ptr_vec(ptr_vec_t *vec, void **init_values, const size_t count);
int add_ptr(ptr_vec_t *vec, void *ptr);
void free_ptr_vec(ptr_vec_t *vec);

#endif /* KOSTAK_UTIL_H */
