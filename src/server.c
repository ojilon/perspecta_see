/* server.c: a tiny HTTP server (one request at a time, localhost only).
   The browser talks HTTP, C answers. That is the whole "bridge". */
#include "server.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET sock_t;
#define SOCK_CLOSE closesocket
#define SEND_FLAGS 0
#else
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
typedef int sock_t;
#define INVALID_SOCKET (-1)
#define SOCK_CLOSE close
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif
#endif

#define REQ_MAX (2 * 1024 * 1024)

/* ---------- small helpers ---------- */
static int hexv(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = tolower(c);
    return (c >= 'a' && c <= 'f') ? c - 'a' + 10 : -1;
}

static void url_decode(const char *s, size_t n, char *out, size_t outsz) {
    size_t o = 0;
    for (size_t i = 0; i < n && o + 1 < outsz; i++) {
        if (s[i] == '%' && i + 2 < n + 0 && hexv(s[i + 1]) >= 0 && hexv(s[i + 2]) >= 0) {
            out[o++] = (char)(hexv(s[i + 1]) * 16 + hexv(s[i + 2]));
            i += 2;
        } else if (s[i] == '+') out[o++] = ' ';
        else out[o++] = s[i];
    }
    out[o] = 0;
}

int query_get(const char *query, const char *key, char *out, size_t outsz) {
    size_t kl = strlen(key);
    const char *p = query;
    while (*p) {
        const char *e = strchr(p, '&');
        size_t len = e ? (size_t)(e - p) : strlen(p);
        if (len > kl && !strncmp(p, key, kl) && p[kl] == '=') {
            url_decode(p + kl + 1, len - kl - 1, out, outsz);
            return 1;
        }
        if (!e) break;
        p = e + 1;
    }
    return 0;
}

static const char *status_text(int s) {
    switch (s) {
    case 200: return "OK";
    case 400: return "Bad Request";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    default:  return "Internal Server Error";
    }
}

static const char *ctype_of(const char *path) {
    const char *d = strrchr(path, '.');
    if (!d) return "application/octet-stream";
    if (!strcmp(d, ".html")) return "text/html; charset=utf-8";
    if (!strcmp(d, ".css"))  return "text/css; charset=utf-8";
    if (!strcmp(d, ".js"))   return "application/javascript; charset=utf-8";
    if (!strcmp(d, ".json")) return "application/json";
    if (!strcmp(d, ".svg"))  return "image/svg+xml";
    return "application/octet-stream";
}

static void send_all(sock_t c, const char *d, size_t n) {
    while (n > 0) {
        int r = (int)send(c, d, (int)(n > 65536 ? 65536 : n), SEND_FLAGS);
        if (r <= 0) return;
        d += r;
        n -= (size_t)r;
    }
}

static void respond(sock_t c, int status, const char *ctype, const char *body, size_t n) {
    char h[320];
    int hl = snprintf(h, sizeof h,
        "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %lu\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n",
        status, status_text(status), ctype, (unsigned long)n);
    send_all(c, h, (size_t)hl);
    if (n) send_all(c, body, n);
}

/* ---------- reading one request ---------- */
static int read_request(sock_t c, Request *rq) {
    static char buf[REQ_MAX + 1];       /* static: too big for the stack */
    size_t got = 0;
    char *hend = NULL;
    while (!hend) {
        if (got >= REQ_MAX) return 0;
        int r = (int)recv(c, buf + got, (int)(REQ_MAX - got), 0);
        if (r <= 0) return 0;
        got += (size_t)r;
        buf[got] = 0;
        hend = strstr(buf, "\r\n\r\n");
    }
    size_t hlen = (size_t)(hend - buf) + 4;

    /* request line: METHOD /path?query HTTP/1.1 */
    char target[1600];
    char *sp1 = strchr(buf, ' ');
    if (!sp1) return 0;
    char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2 || sp2 > hend) return 0;
    size_t ml = (size_t)(sp1 - buf);
    if (ml >= sizeof rq->method) return 0;
    memcpy(rq->method, buf, ml);
    rq->method[ml] = 0;
    size_t tl = (size_t)(sp2 - sp1 - 1);
    if (tl >= sizeof target) return 0;
    memcpy(target, sp1 + 1, tl);
    target[tl] = 0;
    char *q = strchr(target, '?');
    if (q) { *q = 0; snprintf(rq->query, sizeof rq->query, "%s", q + 1); }
    else rq->query[0] = 0;
    url_decode(target, strlen(target), rq->path, sizeof rq->path);

    /* Content-Length header */
    size_t clen = 0;
    for (char *l = strstr(buf, "\r\n"); l && l < hend; l = strstr(l + 2, "\r\n")) {
        const char *k = "content-length:";
        const char *h = l + 2;
        size_t i = 0;
        while (k[i] && tolower((unsigned char)h[i]) == k[i]) i++;
        if (!k[i]) { clen = (size_t)strtoul(h + i, NULL, 10); break; }
    }
    if (clen > REQ_MAX - hlen) return 0;
    while (got < hlen + clen) {
        int r = (int)recv(c, buf + got, (int)(hlen + clen - got), 0);
        if (r <= 0) return 0;
        got += (size_t)r;
    }
    rq->body = buf + hlen;
    rq->body_len = clen;
    return 1;
}

/* ---------- static files (the web/ folder) ---------- */
static void serve_static(sock_t c, const char *web, const char *path) {
    if (!strcmp(path, "/")) path = "/index.html";
    if (strstr(path, "..") || strchr(path, '\\') || strchr(path, ':')) {
        respond(c, 400, "text/plain", "bad path", 8);
        return;
    }
    char full[1100];
    snprintf(full, sizeof full, "%s%s", web, path);
    size_t n = 0;
    char *data = file_read_all(full, &n, 16 * 1024 * 1024);
    if (!data) { respond(c, 404, "text/plain", "not found", 9); return; }
    respond(c, 200, ctype_of(path), data, n);
    free(data);
}

/* ---------- main loop ---------- */
int server_run(int port, const char *web, ApiHandler api) {
#ifdef _WIN32
    WSADATA w;
    if (WSAStartup(MAKEWORD(2, 2), &w)) { fprintf(stderr, "WSAStartup failed\n"); return 1; }
#else
    signal(SIGPIPE, SIG_IGN);
#endif
    sock_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) { fprintf(stderr, "socket() failed\n"); return 1; }
#ifndef _WIN32
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof yes);
#endif
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)port);
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);          /* 127.0.0.1 only: not reachable from the network */
    if (bind(s, (struct sockaddr *)&a, sizeof a) != 0 || listen(s, 16) != 0) {
        fprintf(stderr, "cannot listen on port %d (already running?)\n", port);
        return 1;
    }
    printf("perspecta_see: http://127.0.0.1:%d   (Ctrl+C to stop)\n", port);
    fflush(stdout);

    for (;;) {
        sock_t c = accept(s, NULL, NULL);
        if (c == INVALID_SOCKET) continue;
        /* Browsers open idle "pre-connections"; without a timeout one of them
           would block this single-threaded loop forever. */
#ifdef _WIN32
        DWORD tmo = 800;
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo, sizeof tmo);
#else
        struct timeval tv = { 0, 800000 };
        setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
#endif
        Request rq;
        memset(&rq, 0, sizeof rq);
        if (read_request(c, &rq)) {
            if (!strncmp(rq.path, "/api/", 5)) {
                Buf out;
                buf_init(&out);
                const char *ct = "application/json";
                int st = api(&rq, &out, &ct);
                respond(c, st, ct, out.p ? out.p : "", out.len);
                buf_free(&out);
            } else {
                serve_static(c, web, rq.path);
            }
        }
        SOCK_CLOSE(c);
    }
}