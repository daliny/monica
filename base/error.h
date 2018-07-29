#ifndef BASE_ERROR_H
#define BASE_ERROR_H

#include <errno.h>
#include <string.h>
#include <cstdio>
#include <cstdlib>

namespace monica {
void linux_error(const char* str, int m_errno) {
  fprintf(stderr, "%s: %s\n", str, strerror(m_errno));
  exit(EXIT_FAILURE);
}
}

#endif // BASE_ERROR_H
