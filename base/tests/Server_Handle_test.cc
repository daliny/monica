#include "Server.h"
#include "Handle.h"

#include <stdio.h>

using namespace monica;
int main(int argc, char* argv[]) 
{
  if(argc < 2) {
    fprintf(stderr, "Usage: %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  Server<> server;
  server.work(nullptr, argv[1], 4);
}
