/*
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

///////////////////////////////////////
// Implements:
//   void *memchr(const void *s, int c, size_t n);

#include <stddef.h>
#include <pblibc_private.h>

void *memchr(const void *s, int c, size_t n) {
  const unsigned char *p = (const unsigned char*)s;
  unsigned char ch = (unsigned char)c;
  for (size_t i = 0; i < n; i++) {
    if (p[i] == ch) {
      return (void*)&p[i];
    }
  }
  return NULL;
}
