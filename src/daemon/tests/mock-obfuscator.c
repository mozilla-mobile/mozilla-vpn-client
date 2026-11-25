/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  fprintf(stderr, "Mock obfuscator arguments:");
  for (int i = 1; i < argc; i++) {
    fprintf(stderr, " %s", argv[i]);
  }
  fprintf(stderr, "\n");

  // Parse out the listening port and other special args.
  int listen = 0;
  for (int i = 1; i < argc; i++) {
    const char* s = argv[i];
    const char* next = (i + 1) < argc ? argv[i+1] : "";

    if (strcmp(s, "--listen-port") == 0) {
        listen = atoi(next);
    } else if (strcmp(s, "--nolisten") == 0) {
        listen = -1;
    } else if (strcmp(s, "--exit") == 0) {
        return atoi(next);
    }
  }

  // Select and announce a listen port.
  if (listen == 0) {
    srand(time(0));
    listen = (rand() % 16384 + 49152);
  }
  if (listen > 0) {
    fprintf(stdout, "listening on 127.0.0.1:%d\n", listen);
    fflush(stdout);
  }

  // Wait for a termination signal.
  pause();
  return 0;
}
