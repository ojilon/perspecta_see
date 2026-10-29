/* c/server.c — the C side of the fixture.
 *
 * This comment mentions "/api/comment-only" and "/api/ghost" on purpose: a
 * route is only a route when it appears in code, not in prose.
 */
#include <stdio.h>

/* Not a route either: "/api/in-a-comment". */
const char *route(void) {
    return "/api/real";          /* <- this one is a route definition */
}
