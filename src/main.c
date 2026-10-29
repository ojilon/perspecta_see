/* main.c — start-up.
 *
 * Usage:
 *   perspecta_see [--port N] [--host ADDR] [--web DIR]
 *
 * Defaults are the safe ones: port 8787, host 127.0.0.1 (this machine only),
 * web/ next to the binary. The browser scans whatever folder you type into the
 * box, and only html/css/js/c/h files there can be read or written.
 */
#include "api.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(void) {
    fprintf(stderr,
            "usage: perspecta_see [--port N] [--host ADDR] [--web DIR]\n"
            "  --port  port to listen on            (default 8787)\n"
            "  --host  127.0.0.1 (default) or 0.0.0.0 to expose on the network\n"
            "  --web   folder holding index.html    (default web)\n");
}

int main(int argc, char **argv) {
    int port = 8787;
    const char *host = "127.0.0.1";
    const char *web = "web";

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--port") && i + 1 < argc) port = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--host") && i + 1 < argc) host = argv[++i];
        else if (!strcmp(argv[i], "--web") && i + 1 < argc) web = argv[++i];
        else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) { usage(); return 0; }
        else { usage(); return 1; }
    }
    return server_run(port, host, web, api_handle);
}
