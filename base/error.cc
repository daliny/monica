#include "error.h"

namespace monica {
  void linux_error(const char* str, int m_errno) {
    fprintf(stderr, "%s: %s\n", str, strerror(m_errno));
    exit(EXIT_FAILURE);
  }
}
