#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <hiredis/hiredis.h>

int current_tick() {
    int t = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    t = (int)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return t;
}

int main(int argc, char **argv) {
    unsigned int j, isunix = 0;
    redisContext *c;
    redisReply *reply;

    const char *hostname = "127.0.0.1";
    int port = 6379;
    struct timeval timeout = {1, 500000};
    c = redisConnectWithTimeout(hostname, port, timeout);

    if (c == NULL || c->err) {
        if (c) {
            printf("Connection Error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Can't allocate redis context\n");
        }
        exit(1);
    }

    const char *user = "default";     // 没用户名就置 NULL/空串，并改成一参形式
    const char *pass = "123321";
    redisReply *r = NULL;

    r = user && *user ? redisCommand(c, "AUTH %s %s", user, pass)
                      : redisCommand(c, "AUTH %s", pass);
    if (!r || r->type == REDIS_REPLY_ERROR) {
        fprintf(stderr, "AUTH error: %s\n", r? r->str : c->errstr);
        if (r) freeReplyObject(r); redisFree(c); return 1;
    }
    freeReplyObject(r);

    int num = (argc > 1) ? atoi(argv[1]) : 1000;
    freeReplyObject(redisCommand(c, "DEL counter"));
    int before = current_tick();

    for (int i = 0; i < num; i++) {
        reply = redisCommand(c, "INCR counter");
        if(!reply){
            fprintf(stderr, "NULL reply: %s\n", c->errstr);
            redisFree(c); return 1;
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            fprintf(stderr, "ERR: %s\n", reply->str);
            freeReplyObject(reply); redisFree(c); return 1;
        }
        if (reply->type != REDIS_REPLY_INTEGER) {
            fprintf(stderr, "unexpected type=%d\n", reply->type);
            freeReplyObject(reply); redisFree(c); return 1;
        }
        printf("INCR counter : %11lld\n", reply->integer);
        freeReplyObject(reply);
    }

    int used_time = current_tick() - before;

    printf("Sync Exec %d redis command, Total time: %d ms\n", num, used_time);

    redisFree(c);
    return 0;
}
